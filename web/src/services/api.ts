import axios from 'axios';

const api = axios.create({ baseURL: '/api/v1', timeout: 10000 });

api.interceptors.request.use((config) => {
  const token = localStorage.getItem('pa_token');
  if (token) {
    config.headers.Authorization = `Bearer ${token}`;
  }
  return config;
});

api.interceptors.response.use(
  (resp) => resp,
  (error) => {
    if (error?.response?.status === 401) {
      localStorage.removeItem('pa_token');
      localStorage.removeItem('pa_username');
      window.location.reload();
    }
    return Promise.reject(error);
  }
);

export const deviceApi = {
  list: () => api.get('/devices'),
  register: (data: any) => api.post('/devices', data),
  getStatus: (id: string) => api.get(`/devices/${id}/status`),
  sendCommand: (id: string, data: any) => api.post(`/devices/${id}/command`, data),
};

export const beamApi = {
  form: (data: any) => api.post('/beam/form', data),
  scan: (data: any) => api.post('/beam/scan', data),
  stopScan: () => api.post('/beam/scan/stop'),
  lock: (epc: string, direction: any) => api.post('/beam/lock', { epc, direction }),
  unlock: (epc: string) => api.post('/beam/unlock', { epc }),
  getLocked: () => api.get('/beam/locked'),
};

export const inventoryApi = {
  start: (data: any) => api.post('/inventory/start', data),
  pause: (taskId: string) => api.post(`/inventory/${taskId}/pause`),
  resume: (taskId: string) => api.post(`/inventory/${taskId}/resume`),
  cancel: (taskId: string) => api.post(`/inventory/${taskId}/cancel`),
  getResult: (taskId: string) => api.get(`/inventory/${taskId}/result`),
  listTasks: () => api.get('/inventory/tasks'),
};

export const trajectoryApi = {
  get: (epc: string, params?: any) => api.get(`/trajectory/${epc}`, { params }),
  getStayPoints: (epc: string) => api.get(`/trajectory/${epc}/stay-points`),
  getAnomalies: (params?: any) => api.get('/trajectory/anomalies', { params }),
};

export const configApi = {
  get: () => api.get('/config'),
  update: (data: any) => api.put('/config', data),
  getArray: () => api.get('/config/array'),
  updateArray: (data: any) => api.put('/config/array', data),
};

export interface AssetLedger {
  asset_id: string;
  asset_code: string;
  asset_name: string;
  category: string;
  department: string;
  region: string;
  epc: string | null;
  tag_status: string;
  asset_status: string;
  data_source: string;
  ledger_version: number;
  created_at: string;
  updated_at: string;
}

export interface ImportBatch {
  batch_id: string;
  file_name: string;
  file_format: string;
  strategy: string;
  total_count: number;
  success_count: number;
  failed_count: number;
  skipped_count: number;
  status: string;
  created_at: string;
  completed_at: string;
}

export interface DiffReport {
  report_id: string;
  task_id: string;
  surplus_count: number;
  loss_count: number;
  rebind_count: number;
  error_tag_count: number;
  consistency_rate: number;
  analyzed_at: string;
}

export interface DiffRecord {
  diff_id: string;
  task_id: string;
  diff_type: string;
  asset_code: string | null;
  scanned_epc: string | null;
  ledger_epc: string | null;
  description: string;
  status: string;
}

export interface DiffWorkOrder {
  order_id: string;
  diff_id: string;
  order_type: string;
  handler: string;
  conclusion: string;
  status: string;
  created_at: string;
  completed_at: string;
}

export const ledgerApi = {
  create: (data: any) => api.post('/ledger/assets', data),
  list: (params: any) => api.get('/ledger/assets', { params }),
  update: (id: string, data: any) => api.put(`/ledger/assets/${id}`, data),
  unbindTag: (id: string) => api.post(`/ledger/assets/${id}/unbind-tag`),
  getDictionaries: () => api.get('/ledger/dictionaries'),
};

export const importApi = {
  upload: (file: File, strategy: string) => {
    const formData = new FormData();
    formData.append('file', file);
    formData.append('strategy', strategy);
    return api.post('/ledger/import', formData, { headers: { 'Content-Type': 'multipart/form-data' } });
  },
  getBatch: (batchId: string) => api.get(`/ledger/import/${batchId}`),
  listBatches: (params: any) => api.get('/ledger/import/batches/list', { params }),
  downloadTemplate: () => api.get('/ledger/import/template/download', { responseType: 'blob' }),
};

export const erpSyncApi = {
  trigger: () => api.post('/ledger/erp/sync'),
  listTasks: (params: any) => api.get('/ledger/erp/sync/tasks/list', { params }),
  listConflicts: (status: string) => api.get('/ledger/erp/conflicts/list', { params: { status } }),
  resolveConflict: (ticketId: string, resolution: string) =>
    api.post(`/ledger/erp/conflicts/${ticketId}/resolve`, { resolution }),
};

export const diffApi = {
  analyze: (taskId: string) => api.post(`/diff/analyze/${taskId}`),
  getReport: (taskId: string) => api.get(`/diff/reports/${taskId}`),
  listRecords: (params: any) => api.get('/diff/records', { params }),
};

export const workOrderApi = {
  list: (params: any) => api.get('/diff/work-orders', { params }),
  getDetail: (orderId: string) => api.get(`/diff/work-orders/${orderId}`),
  handleSurplus: (orderId: string, data: any) => api.post(`/diff/work-orders/${orderId}/handle-surplus`, data),
  handleLoss: (orderId: string, data: any) => api.post(`/diff/work-orders/${orderId}/handle-loss`, data),
  bindNewTag: (orderId: string, data: any) => api.post(`/diff/work-orders/${orderId}/bind-new-tag`, data),
  correctTag: (orderId: string, data: any) => api.post(`/diff/work-orders/${orderId}/correct-tag`, data),
};

export interface GroundTruthPoint {
  point_id: string;
  tag_id: string;
  x: number;
  y: number;
  z: number;
  timestamp: number;
  coordinate_system_id: string;
  accuracy: number;
  source: string;
  created_at: string | null;
}

export interface AccuracyReport {
  position_error: number;
  x_error: number;
  y_error: number;
  rmse: number;
  mae: number;
  p50: number;
  p90: number;
  p95: number;
  max_error: number;
  confidence: number;
  sample_count: number;
}

export interface ComparisonRecord {
  record_id: string;
  session_id: string;
  gt_point_id: string;
  estimated_x: number | null;
  estimated_y: number | null;
  estimated_z: number | null;
  error_x: number | null;
  error_y: number | null;
  error_z: number | null;
  error_total: number | null;
  skipped: boolean;
  skip_reason: string | null;
}

export interface CalibrationSession {
  session_id: string;
  status: string;
  localization_mode: string;
  coordinate_system_id: string;
  ground_truth_count: number;
  target_accuracy: number;
  calibration_version: number | null;
  start_timestamp: number | null;
  completion_timestamp: number | null;
  repeatability_verified: boolean;
}

export interface CoordinateSystem {
  id: string;
  version: number;
  parent_id: string | null;
  origin_x: number;
  origin_y: number;
  origin_z: number;
  rotation: number;
  unit: string;
  is_deprecated: boolean;
}

export interface BenchmarkScenario {
  scenario_id: string;
  name: string;
  description: string;
  algorithms: string[];
  reader_count: number;
  is_preset: boolean;
}

export interface BenchmarkReport {
  report_id: string;
  scenarios: any[];
  algorithms: string[];
  comparison_table: any;
  generated_at: number;
}

export interface SpatialGraphData {
  nodes: any[];
  edges: any[];
  timestamp: number;
}

export interface FactoryMapData {
  spatial_graph: SpatialGraphData | null;
  node_status: Record<string, string>;
  update_timestamp: number;
}

export const spatialTruthApi = {
  addGroundTruth: (data: any) => api.post('/spatial-truth/ground-truth', data),
  listGroundTruth: (params: any) => api.get('/spatial-truth/ground-truth', { params }),
  deleteGroundTruth: (pointId: string) => api.delete(`/spatial-truth/ground-truth/${pointId}`),
  createCalibrationSession: (data: any) => api.post('/spatial-truth/calibration/sessions', data),
  getCalibrationSession: (sessionId: string) => api.get(`/spatial-truth/calibration/sessions/${sessionId}`),
  listCalibrationVersions: (params: any) => api.get('/spatial-truth/calibration/versions', { params }),
};

export const coordinateSystemApi = {
  create: (data: any) => api.post('/spatial-truth/coordinate-systems', data),
  list: () => api.get('/spatial-truth/coordinate-systems'),
  transform: (data: any) => api.post('/spatial-truth/coordinate-systems/transform', data),
};

export const spatialBenchmarkApi = {
  listScenarios: () => api.get('/spatial-benchmark/scenarios'),
  runBenchmark: (data: any) => api.post('/spatial-benchmark/run', data),
  getReport: (reportId: string) => api.get(`/spatial-benchmark/reports/${reportId}`),
  exportReport: (reportId: string, format: string = 'json') =>
    api.get(`/spatial-benchmark/reports/${reportId}/export`, { params: { format } }),
};

export const spatialGraphApi = {
  getSpatialGraph: () => api.get('/spatial-truth/spatial-graph'),
  getFactoryMap: (zoneId?: string) => api.get('/spatial-truth/factory-map', { params: { zone_id: zoneId } }),
  getFactoryMapByZone: (zoneId: string) => api.get(`/spatial-truth/factory-map/zone/${zoneId}`),
};

export const spatialMapWs = {
  connect: () => {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    return new WebSocket(`${protocol}//${window.location.host}/api/v1/spatial-truth/ws/spatial-map`);
  },
};

export default api;
