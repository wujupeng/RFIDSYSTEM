import { useState, useEffect } from 'react';
import { Card, Tree, Button, Modal, Form, Input, InputNumber, Select, message, Space, Descriptions, Tag } from 'antd';
import { PlusOutlined } from '@ant-design/icons';
import { coordinateSystemApi, CoordinateSystem } from '../services/api';

const UNIT_OPTIONS = [
  { label: '米 (meter)', value: 'meter' },
  { label: '厘米 (centimeter)', value: 'centimeter' },
  { label: '毫米 (millimeter)', value: 'millimeter' },
];

export default function CoordinateSystemPage() {
  const [systems, setSystems] = useState<CoordinateSystem[]>([]);

  const [addOpen, setAddOpen] = useState(false);
  const [transformOpen, setTransformOpen] = useState(false);
  const [transformResult, setTransformResult] = useState<any>(null);
  const [form] = Form.useForm();
  const [transformForm] = Form.useForm();

  const fetchData = async () => {
    try {
      const res = await coordinateSystemApi.list();
      setSystems(res.data || []);
    } catch (e: any) {
      message.error('加载失败');
    }
  };

  useEffect(() => { fetchData(); }, []);

  const handleAdd = async () => {
    const values = await form.validateFields();
    try {
      await coordinateSystemApi.create(values);
      message.success('创建成功');
      setAddOpen(false);
      form.resetFields();
      fetchData();
    } catch (e: any) {
      message.error(e?.response?.data?.detail || '创建失败');
    }
  };

  const handleTransform = async () => {
    const values = await transformForm.validateFields();
    try {
      const res = await coordinateSystemApi.transform(values);
      setTransformResult(res.data);
      message.success('变换成功');
    } catch (e: any) {
      message.error(e?.response?.data?.detail || '变换失败');
    }
  };

  const buildTree = () => {
    const root = systems.find(s => !s.parent_id) || systems[0];
    if (!root) return [];

    const buildChildren = (parentId: string): any[] => {
      return systems
        .filter(s => s.parent_id === parentId)
        .map(s => ({
          key: s.id,
          title: <Space><span>{s.id}</span><Tag>v{s.version}</Tag><Tag>{s.unit}</Tag></Space>,
          children: buildChildren(s.id),
        }));
    };

    return [{
      key: root.id,
      title: <Space><span>{root.id}</span><Tag>v{root.version}</Tag><Tag color="green">根</Tag></Space>,
      children: buildChildren(root.id),
    }];
  };

  const csOptions = systems.map(s => ({ label: `${s.id} (v${s.version})`, value: s.id }));

  return (
    <div>
      <Card title="坐标系层级管理" extra={
        <Space>
          <Button type="primary" icon={<PlusOutlined />} onClick={() => setAddOpen(true)}>创建坐标系</Button>
          <Button onClick={() => setTransformOpen(true)}>坐标变换测试</Button>
        </Space>
      }>
        <Tree treeData={buildTree()} defaultExpandAll />
      </Card>

      {systems.length > 0 && (
        <Card title="坐标系列表" style={{ marginTop: 16 }}>
          {systems.map(cs => (
            <Descriptions key={cs.id + cs.version} title={cs.id} size="small" bordered column={4} style={{ marginBottom: 16 }}>
              <Descriptions.Item label="版本">{cs.version}</Descriptions.Item>
              <Descriptions.Item label="父坐标系">{cs.parent_id || '(根)'}</Descriptions.Item>
              <Descriptions.Item label="单位">{cs.unit}</Descriptions.Item>
              <Descriptions.Item label="旋转">{cs.rotation} rad</Descriptions.Item>
              <Descriptions.Item label="Origin X">{cs.origin_x}</Descriptions.Item>
              <Descriptions.Item label="Origin Y">{cs.origin_y}</Descriptions.Item>
              <Descriptions.Item label="Origin Z">{cs.origin_z}</Descriptions.Item>
              <Descriptions.Item label="废弃"><Tag color={cs.is_deprecated ? 'red' : 'green'}>{cs.is_deprecated ? '是' : '否'}</Tag></Descriptions.Item>
            </Descriptions>
          ))}
        </Card>
      )}

      <Modal title="创建坐标系" open={addOpen} onOk={handleAdd} onCancel={() => setAddOpen(false)} width={500}>
        <Form form={form} layout="vertical" initialValues={{ unit: 'meter', parent_id: 'global' }}>
          <Form.Item name="id" label="坐标系ID" rules={[{ required: true }]}><Input /></Form.Item>
          <Form.Item name="parent_id" label="父坐标系"><Select options={csOptions} allowClear /></Form.Item>
          <Space>
            <Form.Item name="origin_x" label="Origin X"><InputNumber step={0.1} /></Form.Item>
            <Form.Item name="origin_y" label="Origin Y"><InputNumber step={0.1} /></Form.Item>
            <Form.Item name="origin_z" label="Origin Z"><InputNumber step={0.1} /></Form.Item>
          </Space>
          <Form.Item name="rotation" label="旋转(弧度)"><InputNumber step={0.01} /></Form.Item>
          <Form.Item name="unit" label="单位"><Select options={UNIT_OPTIONS} /></Form.Item>
        </Form>
      </Modal>

      <Modal title="坐标变换测试" open={transformOpen} onOk={handleTransform} onCancel={() => { setTransformOpen(false); setTransformResult(null); }} width={500}>
        <Form form={transformForm} layout="vertical">
          <Form.Item name="source_cs_id" label="源坐标系" rules={[{ required: true }]}><Select options={csOptions} /></Form.Item>
          <Form.Item name="target_cs_id" label="目标坐标系" rules={[{ required: true }]}><Select options={csOptions} /></Form.Item>
          <Space>
            <Form.Item name="x" label="X" rules={[{ required: true }]}><InputNumber step={0.001} /></Form.Item>
            <Form.Item name="y" label="Y" rules={[{ required: true }]}><InputNumber step={0.001} /></Form.Item>
            <Form.Item name="z" label="Z"><InputNumber step={0.001} /></Form.Item>
          </Space>
        </Form>
        {transformResult && (
          <Descriptions title="变换结果" bordered size="small" column={2}>
            <Descriptions.Item label="X">{transformResult.x?.toFixed(6)}</Descriptions.Item>
            <Descriptions.Item label="Y">{transformResult.y?.toFixed(6)}</Descriptions.Item>
            <Descriptions.Item label="Z">{transformResult.z?.toFixed(6)}</Descriptions.Item>
            <Descriptions.Item label="往返一致">{transformResult.round_trip_valid ? <Tag color="green">通过</Tag> : <Tag color="red">失败</Tag>}</Descriptions.Item>
          </Descriptions>
        )}
      </Modal>
    </div>
  );
}