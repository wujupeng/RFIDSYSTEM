import { useState, useEffect, useRef } from 'react';
import { Card, Switch, Space, Tag, Empty, Spin, message } from 'antd';
import { spatialGraphApi, spatialMapWs, SpatialGraphData, FactoryMapData } from '../services/api';

const STATUS_COLORS: Record<string, string> = {
  NORMAL: '#52c41a', UNCERTAIN: '#faad14', RISK: '#ff4d4f',
};

const STATUS_LABELS: Record<string, string> = {
  NORMAL: '正常', UNCERTAIN: '不确定', RISK: '风险',
};

export default function SpatialMap() {
  const [graph, setGraph] = useState<SpatialGraphData | null>(null);
  const [factoryMap, setFactoryMap] = useState<FactoryMapData | null>(null);
  const [loading, setLoading] = useState(false);
  const [layers, setLayers] = useState({ heatmap: true, aoa: true, trail: true, flow: true, labels: true });
  const wsRef = useRef<WebSocket | null>(null);

  const fetchData = async () => {
    setLoading(true);
    try {
      const [graphRes, mapRes] = await Promise.all([
        spatialGraphApi.getSpatialGraph(),
        spatialGraphApi.getFactoryMap(),
      ]);
      setGraph(graphRes.data);
      setFactoryMap(mapRes.data);
    } catch (e: any) {
      message.error('加载地图失败');
    }
    setLoading(false);
  };

  useEffect(() => {
    fetchData();

    try {
      const ws = spatialMapWs.connect();
      ws.onmessage = (event) => {
        try {
          const msg = JSON.parse(event.data);
          if (msg.event_type === 'tag_moved' || msg.event_type === 'reader_online' || msg.event_type === 'reader_offline') {
            fetchData();
          }
        } catch { }
      };
      wsRef.current = ws;
    } catch { }

    return () => { wsRef.current?.close(); };
  }, []);

  const nodes = graph?.nodes || [];
  const edges = graph?.edges || [];
  const nodeStatus = factoryMap?.node_status || {};

  const readerNodes = nodes.filter((n: any) => n.node_type === 'reader');
  const tagNodes = nodes.filter((n: any) => n.node_type === 'tag');

  return (
    <div>
      <Card title="工厂空间地图" extra={
        <Space>
          <span>热力场</span><Switch size="small" checked={layers.heatmap} onChange={(v) => setLayers({ ...layers, heatmap: v })} />
          <span>AoA</span><Switch size="small" checked={layers.aoa} onChange={(v) => setLayers({ ...layers, aoa: v })} />
          <span>轨迹</span><Switch size="small" checked={layers.trail} onChange={(v) => setLayers({ ...layers, trail: v })} />
          <span>流量</span><Switch size="small" checked={layers.flow} onChange={(v) => setLayers({ ...layers, flow: v })} />
          <span>标签</span><Switch size="small" checked={layers.labels} onChange={(v) => setLayers({ ...layers, labels: v })} />
        </Space>
      }>
        {loading ? <Spin tip="加载中..." /> : (
          nodes.length === 0 ? <Empty description="暂无空间图数据" /> : (
            <div style={{ position: 'relative', width: '100%', height: 500, background: '#f5f5f5', border: '1px solid #d9d9d9', borderRadius: 8, overflow: 'hidden' }}>
              <svg width="100%" height="100%" style={{ position: 'absolute', top: 0, left: 0 }}>
                {edges.map((edge: any, i: number) => {
                  const from = nodes.find((n: any) => n.node_id === edge.from_node);
                  const to = nodes.find((n: any) => n.node_id === edge.to_node);
                  if (!from || !to) return null;
                  const fx = parseFloat(from.attributes?.x || '0');
                  const fy = parseFloat(from.attributes?.y || '0');
                  const tx = parseFloat(to.attributes?.x || '0');
                  const ty = parseFloat(to.attributes?.y || '0');
                  return <line key={i} x1={fx * 5} y1={fy * 5} x2={tx * 5} y2={ty * 5} stroke="#ccc" strokeWidth={1} />;
                })}
                {nodes.map((node: any) => {
                  const x = parseFloat(node.attributes?.x || '0') * 5;
                  const y = parseFloat(node.attributes?.y || '0') * 5;
                  const status = nodeStatus[node.node_id];
                  const color = node.node_type === 'reader' ? '#1890ff' : (status ? STATUS_COLORS[status] : '#999');
                  const r = node.node_type === 'reader' ? 8 : 5;
                  return (
                    <g key={node.node_id}>
                      <circle cx={x} cy={y} r={r} fill={color} stroke="#fff" strokeWidth={1} />
                      {layers.labels && (
                        <text x={x + r + 2} y={y + 3} fontSize={10} fill="#333">{node.node_id}</text>
                      )}
                    </g>
                  );
                })}
              </svg>
            </div>
          )
        )}
      </Card>

      <Card title="节点状态" style={{ marginTop: 16 }}>
        <Space wrap>
          {Object.entries(nodeStatus).map(([id, status]) => (
            <Tag key={id} color={STATUS_COLORS[status] || 'default'}>
              {id}: {STATUS_LABELS[status] || status}
            </Tag>
          ))}
          {Object.keys(nodeStatus).length === 0 && <Empty description="暂无状态数据" />}
        </Space>
      </Card>

      <Card title="统计" style={{ marginTop: 16 }}>
        <Space size="large">
          <Tag color="blue">Reader: {readerNodes.length}</Tag>
          <Tag color="cyan">Tag: {tagNodes.length}</Tag>
          <Tag color="purple">边: {edges.length}</Tag>
          <Tag color="orange">总节点: {nodes.length}</Tag>
        </Space>
      </Card>
    </div>
  );
}