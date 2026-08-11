import { useState, useEffect } from 'react';
import { Card, Table, Button, Checkbox, Space, message, Tag, Spin, Descriptions } from 'antd';
import { spatialBenchmarkApi, BenchmarkScenario, BenchmarkReport } from '../services/api';

const ALGO_OPTIONS = [
  { label: 'RSSI Only', value: 'rssi_only' },
  { label: 'RSSI Triangulation', value: 'rssi_triangulation' },
  { label: 'RSSI Phase', value: 'rssi_phase' },
  { label: 'AoA', value: 'aoa' },
  { label: 'Beamforming', value: 'beamforming' },
];

export default function SpatialBenchmark() {
  const [scenarios, setScenarios] = useState<BenchmarkScenario[]>([]);
  const [selectedScenarios, setSelectedScenarios] = useState<string[]>([]);
  const [selectedAlgos, setSelectedAlgos] = useState<string[]>([]);
  const [running, setRunning] = useState(false);
  const [report, setReport] = useState<BenchmarkReport | null>(null);

  useEffect(() => {
    const fetchScenarios = async () => {
      try {
        const res = await spatialBenchmarkApi.listScenarios();
        setScenarios(res.data.scenarios || []);
      } catch (e: any) {
        message.error('加载场景失败');
      }
    };
    fetchScenarios();
  }, []);

  const handleRun = async () => {
    if (selectedScenarios.length === 0 || selectedAlgos.length === 0) {
      message.warning('请选择场景和算法');
      return;
    }
    setRunning(true);
    try {
      const res = await spatialBenchmarkApi.runBenchmark({
        scenario_ids: selectedScenarios,
        algorithms: selectedAlgos,
      });
      setReport(res.data);
      message.success('Benchmark 完成');
    } catch (e: any) {
      message.error(e?.response?.data?.detail || '运行失败');
    }
    setRunning(false);
  };

  const handleExport = async (format: string) => {
    if (!report) return;
    try {
      const res = await spatialBenchmarkApi.exportReport(report.report_id, format);
      if (format === 'csv') {
        const blob = new Blob([res.data], { type: 'text/csv' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url; a.download = `${report.report_id}.csv`; a.click();
        URL.revokeObjectURL(url);
      } else {
        const blob = new Blob([JSON.stringify(res.data, null, 2)], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url; a.download = `${report.report_id}.json`; a.click();
        URL.revokeObjectURL(url);
      }
    } catch (e: any) {
      message.error('导出失败');
    }
  };

  const tableColumns = scenarios.length > 0
    ? [
        { title: '算法', dataIndex: 'algorithm', key: 'algorithm', fixed: 'left' as const, width: 150 },
        ...scenarios.map(s => ({
          title: `场景 ${s.scenario_id}: ${s.name}`,
          key: s.scenario_id,
          render: (_: any, row: any) => {
            const cell = row.cells?.[s.scenario_id];
            if (!cell) return '-';
            if (cell.na) return <Tag color="default">N/A</Tag>;
            return (
              <Space direction="vertical" size={0}>
                <span>P50: {cell.p50?.toFixed(4)}</span>
                <span>P90: {cell.p90?.toFixed(4)}</span>
                <span>P95: {cell.p95?.toFixed(4)}</span>
                <span>Max: {cell.max_error?.toFixed(4)}</span>
              </Space>
            );
          },
        })),
      ]
    : [];

  const tableData = report?.comparison_table?.rows?.map((row: any) => ({
    key: row.algorithm,
    algorithm: row.algorithm,
    cells: row.cells,
  })) || [];

  return (
    <div>
      <Card title="Benchmark 场景列表">
        <div style={{ display: 'flex', flexWrap: 'wrap', gap: 16 }}>
          {scenarios.map(s => (
            <Card key={s.scenario_id} size="small" style={{ width: 250 }} title={`场景 ${s.scenario_id}: ${s.name}`}>
              <p>{s.description}</p>
              <p>算法: {s.algorithms.join(', ')}</p>
              <p>Reader数: {s.reader_count}</p>
              <Tag color="blue">预置场景</Tag>
            </Card>
          ))}
        </div>
      </Card>

      <Card title="运行 Benchmark" style={{ marginTop: 16 }}>
        <Space direction="vertical" style={{ width: '100%' }}>
          <div>
            <span>选择场景: </span>
            <Checkbox.Group options={scenarios.map(s => ({ label: s.scenario_id, value: s.scenario_id }))} value={selectedScenarios} onChange={(vals) => setSelectedScenarios(vals as string[])} />
          </div>
          <div>
            <span>选择算法: </span>
            <Checkbox.Group options={ALGO_OPTIONS} value={selectedAlgos} onChange={(vals) => setSelectedAlgos(vals as string[])} />
          </div>
          <Space>
            <Button type="primary" onClick={handleRun} loading={running}>运行 Benchmark</Button>
            {report && (
              <>
                <Button onClick={() => handleExport('json')}>导出 JSON</Button>
                <Button onClick={() => handleExport('csv')}>导出 CSV</Button>
              </>
            )}
          </Space>
        </Space>
      </Card>

      {running && <Spin tip="运行中..." style={{ display: 'block', marginTop: 24 }} />}

      {report && !running && (
        <Card title="对比表" style={{ marginTop: 16 }}>
          <Descriptions size="small" column={3} style={{ marginBottom: 16 }}>
            <Descriptions.Item label="报告ID">{report.report_id}</Descriptions.Item>
            <Descriptions.Item label="生成时间">{new Date(report.generated_at).toLocaleString()}</Descriptions.Item>
            <Descriptions.Item label="场景数">{report.scenarios?.length || 0}</Descriptions.Item>
          </Descriptions>
          <Table columns={tableColumns} dataSource={tableData} pagination={false} scroll={{ x: 'max-content' }} />
        </Card>
      )}
    </div>
  );
}