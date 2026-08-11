import { useState, useEffect } from 'react';
import { Card, Table, Button, Modal, Form, Input, InputNumber, Select, Radio, message, Steps, Tag, Space, Descriptions, Divider } from 'antd';
import { PlusOutlined, DeleteOutlined, ExperimentOutlined } from '@ant-design/icons';
import { spatialTruthApi, coordinateSystemApi, GroundTruthPoint, AccuracyReport, ComparisonRecord, CalibrationSession } from '../services/api';

const SOURCE_OPTIONS = [
  { label: '手工录入', value: 'manual' },
  { label: '全站仪', value: 'total_station' },
  { label: '激光跟踪', value: 'laser' },
  { label: '参考点', value: 'reference_point' },
  { label: '已知标签', value: 'known_tag' },
];

const SOURCE_ACCURACY: Record<string, number> = {
  known_tag: 0.01, total_station: 0.005, laser: 0.02, reference_point: 0.05, manual: 0.10,
};

const MODE_OPTIONS = [
  { label: 'RSSI Only', value: 'rssi_only' },
  { label: 'RSSI Triangulation', value: 'rssi_triangulation' },
  { label: 'RSSI Phase', value: 'rssi_phase' },
  { label: 'AoA', value: 'aoa' },
  { label: 'Beamforming', value: 'beamforming' },
];

export default function SpatialTruth() {
  const [data, setData] = useState<GroundTruthPoint[]>([]);
  const [total, setTotal] = useState(0);
  const [loading, setLoading] = useState(false);
  const [page, setPage] = useState(1);
  const [addModalOpen, setAddModalOpen] = useState(false);
  const [calModalOpen, setCalModalOpen] = useState(false);
  const [csOptions, setCsOptions] = useState<{label:string,value:string}[]>([]);
  const [sessionDetail, setSessionDetail] = useState<any>(null);
  const [versions, setVersions] = useState<any[]>([]);
  const [form] = Form.useForm();
  const [calForm] = Form.useForm();

  const fetchData = async () => {
    setLoading(true);
    try {
      const res = await spatialTruthApi.listGroundTruth({ offset: (page - 1) * 10, limit: 10 });
      setData(res.data.items || []);
      setTotal(res.data.total || 0);
    } catch (e: any) {
      message.error(e?.response?.data?.detail || '加载失败');
    }
    setLoading(false);
  };

  const fetchCsOptions = async () => {
    try {
      const res = await coordinateSystemApi.list();
      const items = res.data || [];
      setCsOptions(items.map((cs: any) => ({ label: `${cs.id} (v${cs.version})`, value: cs.id })));
    } catch { setCsOptions([{ label: 'global', value: 'global' }]); }
  };

  const fetchVersions = async () => {
    try {
      const res = await spatialTruthApi.listCalibrationVersions({ offset: 0, limit: 20 });
      setVersions(res.data.items || []);
    } catch { }
  };

  useEffect(() => { fetchData(); fetchCsOptions(); fetchVersions(); }, [page]);

  const handleAdd = async () => {
    const values = await form.validateFields();
    try {
      await spatialTruthApi.addGroundTruth({
        ...values,
        accuracy: values.accuracy ?? SOURCE_ACCURACY[values.source] ?? 0.1,
        z: values.z ?? 0,
        coordinate_system_id: values.coordinate_system_id || 'global',
      });
      message.success('录入成功');
      setAddModalOpen(false);
      form.resetFields();
      fetchData();
    } catch (e: any) {
      message.error(e?.response?.data?.detail || '录入失败');
    }
  };

  const handleDelete = async (pointId: string) => {
    try {
      await spatialTruthApi.deleteGroundTruth(pointId);
      message.success('删除成功');
      fetchData();
    } catch (e: any) {
      message.error(e?.response?.data?.detail || '删除失败');
    }
  };

  const handleCreateSession = async () => {
    const values = await calForm.validateFields();
    try {
      const res = await spatialTruthApi.createCalibrationSession(values);
      message.success('标定会话已创建');
      setCalModalOpen(false);
      calForm.resetFields();
      const detail = await spatialTruthApi.getCalibrationSession(res.data.session_id);
      setSessionDetail(detail.data);
      fetchVersions();
    } catch (e: any) {
      message.error(e?.response?.data?.detail || '创建失败');
    }
  };

  const sourceColor: Record<string, string> = {
    manual: 'blue', total_station: 'green', laser: 'orange', reference_point: 'purple', known_tag: 'cyan',
  };

  const columns = [
    { title: '点ID', dataIndex: 'point_id', key: 'point_id', width: 180 },
    { title: '标签ID', dataIndex: 'tag_id', key: 'tag_id', width: 140 },
    { title: 'X', dataIndex: 'x', key: 'x', width: 80 },
    { title: 'Y', dataIndex: 'y', key: 'y', width: 80 },
    { title: 'Z', dataIndex: 'z', key: 'z', width: 80 },
    { title: '精度(m)', dataIndex: 'accuracy', key: 'accuracy', width: 80, render: (v: number) => <Tag color={v < 0.02 ? 'green' : v < 0.1 ? 'orange' : 'red'}>{v}</Tag> },
    { title: '来源', dataIndex: 'source', key: 'source', width: 100, render: (v: string) => <Tag color={sourceColor[v] || 'default'}>{v}</Tag> },
    { title: '坐标系', dataIndex: 'coordinate_system_id', key: 'coordinate_system_id', width: 100 },
    { title: '操作', key: 'action', render: (_: any, record: GroundTruthPoint) => (
      <Button size="small" danger icon={<DeleteOutlined />} onClick={() => handleDelete(record.point_id)}>删除</Button>
    )},
  ];

  const report = sessionDetail?.report as AccuracyReport | null;
  const session = sessionDetail?.session as CalibrationSession | null;
  const records = (sessionDetail?.comparison_records || []) as ComparisonRecord[];

  const statusSteps = [
    { title: 'Created' }, { title: 'Running' }, { title: 'Analyzing' }, { title: 'Completed' },
  ];
  const statusIndex = session ? { created: 0, running: 1, analyzing: 2, completed: 3, failed: 3 }[session.status] ?? 0 : 0;

  return (
    <div>
      <Card title="Ground Truth 基准点管理" extra={
        <Space>
          <Button type="primary" icon={<PlusOutlined />} onClick={() => setAddModalOpen(true)}>录入基准点</Button>
          <Button icon={<ExperimentOutlined />} onClick={() => setCalModalOpen(true)}>发起标定会话</Button>
        </Space>
      }>
        <Table columns={columns} dataSource={data} rowKey="point_id" loading={loading}
          pagination={{ current: page, total, pageSize: 10, onChange: setPage }} />
      </Card>

      {sessionDetail && (
        <Card title="标定会话详情" style={{ marginTop: 16 }}>
          <Steps current={statusIndex} items={statusSteps} status={session?.status === 'failed' ? 'error' : 'finish'} />
          {report && (
            <>
              <Descriptions title="精度报告（十指标）" bordered column={5} style={{ marginTop: 16 }}>
                <Descriptions.Item label="Position Error">{report.position_error.toFixed(4)}</Descriptions.Item>
                <Descriptions.Item label="X Error">{report.x_error.toFixed(4)}</Descriptions.Item>
                <Descriptions.Item label="Y Error">{report.y_error.toFixed(4)}</Descriptions.Item>
                <Descriptions.Item label="RMSE">{report.rmse.toFixed(4)}</Descriptions.Item>
                <Descriptions.Item label="MAE">{report.mae.toFixed(4)}</Descriptions.Item>
                <Descriptions.Item label="P50">{report.p50.toFixed(4)}</Descriptions.Item>
                <Descriptions.Item label="P90">{report.p90.toFixed(4)}</Descriptions.Item>
                <Descriptions.Item label="P95">{report.p95.toFixed(4)}</Descriptions.Item>
                <Descriptions.Item label="Max Error">{report.max_error.toFixed(4)}</Descriptions.Item>
                <Descriptions.Item label="Confidence">{(report.confidence * 100).toFixed(1)}%</Descriptions.Item>
              </Descriptions>
              <Divider />
              <Table title={() => '比对记录'} columns={[
                { title: '记录ID', dataIndex: 'record_id', key: 'record_id' },
                { title: 'GT点ID', dataIndex: 'gt_point_id', key: 'gt_point_id' },
                { title: '估计X', dataIndex: 'estimated_x', key: 'estimated_x' },
                { title: '估计Y', dataIndex: 'estimated_y', key: 'estimated_y' },
                { title: '误差', dataIndex: 'error_total', key: 'error_total', render: (v: number) => v?.toFixed(4) },
                { title: '跳过', dataIndex: 'skipped', key: 'skipped', render: (v: boolean) => v ? <Tag color="red">跳过</Tag> : <Tag color="green">正常</Tag> },
              ]} dataSource={records} rowKey="record_id" pagination={{ pageSize: 10 }} />
            </>
          )}
        </Card>
      )}

      {versions.length > 0 && (
        <Card title="标定版本历史" style={{ marginTop: 16 }}>
          <Table columns={[
            { title: '版本号', dataIndex: 'version', key: 'version' },
            { title: '时间', dataIndex: 'timestamp', key: 'timestamp' },
            { title: '会话ID', dataIndex: 'session_id', key: 'session_id' },
          ]} dataSource={versions} rowKey="version" pagination={{ pageSize: 10 }} />
        </Card>
      )}

      <Modal title="录入 Ground Truth 基准点" open={addModalOpen} onOk={handleAdd} onCancel={() => setAddModalOpen(false)} width={600}>
        <Form form={form} layout="vertical">
          <Form.Item name="point_id" label="点ID" rules={[{ required: true }]}><Input /></Form.Item>
          <Form.Item name="tag_id" label="标签ID" rules={[{ required: true }]}><Input /></Form.Item>
          <Form.Item name="source" label="来源" rules={[{ required: true }]}>
            <Radio.Group options={SOURCE_OPTIONS} onChange={(e) => form.setFieldValue('accuracy', SOURCE_ACCURACY[e.target.value])} />
          </Form.Item>
          <Form.Item name="coordinate_system_id" label="坐标系"><Select options={csOptions} placeholder="global" /></Form.Item>
          <Space>
            <Form.Item name="x" label="X" rules={[{ required: true }]}><InputNumber step={0.001} /></Form.Item>
            <Form.Item name="y" label="Y" rules={[{ required: true }]}><InputNumber step={0.001} /></Form.Item>
            <Form.Item name="z" label="Z"><InputNumber step={0.001} /></Form.Item>
          </Space>
          <Form.Item name="accuracy" label="精度(m)"><InputNumber step={0.001} min={0} /></Form.Item>
          <Form.Item name="timestamp" label="时间戳(ms)" rules={[{ required: true }]}><InputNumber /></Form.Item>
        </Form>
      </Modal>

      <Modal title="发起标定会话" open={calModalOpen} onOk={handleCreateSession} onCancel={() => setCalModalOpen(false)} width={500}>
        <Form form={calForm} layout="vertical" initialValues={{ localization_mode: 'rssi_triangulation', coordinate_system_id: 'global', target_accuracy: 0.1 }}>
          <Form.Item name="localization_mode" label="定位算法" rules={[{ required: true }]}>
            <Select options={MODE_OPTIONS} />
          </Form.Item>
          <Form.Item name="coordinate_system_id" label="坐标系"><Select options={csOptions} /></Form.Item>
          <Form.Item name="target_accuracy" label="目标精度(m)"><InputNumber step={0.01} min={0} /></Form.Item>
          <Form.Item name="source_filter" label="来源过滤"><Select options={SOURCE_OPTIONS} allowClear /></Form.Item>
        </Form>
      </Modal>
    </div>
  );
}