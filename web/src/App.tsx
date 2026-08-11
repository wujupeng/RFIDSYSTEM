import { BrowserRouter, Routes, Route, Navigate, useNavigate, useLocation } from 'react-router-dom';
import { Layout, Menu, Button, Space, Typography } from 'antd';
import {
  DashboardOutlined,
  ApiOutlined,
  AimOutlined,
  ScanOutlined,
  NodeIndexOutlined,
  SettingOutlined,
  LogoutOutlined,
  DatabaseOutlined,
  ImportOutlined,
  CloudSyncOutlined,
  DiffOutlined,
  FileSearchOutlined,
  ApartmentOutlined,
  EnvironmentOutlined,
  BlockOutlined,
} from '@ant-design/icons';
import { useState, useEffect } from 'react';
import Dashboard from './pages/Dashboard';
import DeviceManagement from './pages/DeviceManagement';
import BeamControl from './pages/BeamControl';
import Inventory from './pages/Inventory';
import Trajectory from './pages/Trajectory';
import Settings from './pages/Settings';
import Login from './pages/Login';
import AssetLedgerPage from './pages/AssetLedger';
import LedgerImport from './pages/LedgerImport';
import ErpSync from './pages/ErpSync';
import DiffAnalysis from './pages/DiffAnalysis';
import DiffWorkOrders from './pages/DiffWorkOrders';
import SpatialTruth from './pages/SpatialTruth';
import SpatialBenchmark from './pages/SpatialBenchmark';
import SpatialMap from './pages/SpatialMap';
import CoordinateSystemPage from './pages/CoordinateSystem';

const { Header, Sider, Content } = Layout;
const { Text } = Typography;

const menuItems = [
  { key: '/dashboard', icon: <DashboardOutlined />, label: '系统概览' },
  { key: '/devices', icon: <ApiOutlined />, label: '设备管理' },
  { key: '/beam', icon: <AimOutlined />, label: '波束控制' },
  { key: '/inventory', icon: <ScanOutlined />, label: '盘点管理' },
  { key: '/trajectory', icon: <NodeIndexOutlined />, label: '轨迹分析' },
  { key: '/ledger', icon: <DatabaseOutlined />, label: '资产台账' },
  { key: '/ledger-import', icon: <ImportOutlined />, label: '批量导入' },
  { key: '/erp-sync', icon: <CloudSyncOutlined />, label: 'ERP同步' },
  { key: '/diff-analysis', icon: <DiffOutlined />, label: '差异分析' },
  { key: '/diff-orders', icon: <FileSearchOutlined />, label: '差异工单' },
  { key: '/spatial-truth', icon: <AimOutlined />, label: '空间真值与标定' },
  { key: '/spatial-benchmark', icon: <ApartmentOutlined />, label: '算法基准测试' },
  { key: '/spatial-map', icon: <EnvironmentOutlined />, label: '工厂空间地图' },
  { key: '/coordinate-system', icon: <BlockOutlined />, label: '坐标系管理' },
  { key: '/settings', icon: <SettingOutlined />, label: '系统设置' },
];

function AppLayout({ username, onLogout }: { username: string; onLogout: () => void }) {
  const [collapsed, setCollapsed] = useState(false);
  const navigate = useNavigate();
  const location = useLocation();

  return (
    <Layout style={{ minHeight: '100vh' }}>
      <Sider collapsible collapsed={collapsed} onCollapse={setCollapsed} width={220}>
        <div style={{ height: 48, margin: 8, color: '#fff', textAlign: 'center', lineHeight: '48px', fontSize: 16 }}>
          {collapsed ? 'PA' : 'RFID相控阵系统'}
        </div>
        <Menu theme="dark" selectedKeys={[location.pathname]} items={menuItems} onClick={(e) => navigate(e.key)} />
      </Sider>
      <Layout>
        <Header style={{ padding: '0 24px', background: '#fff', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
          <h2 style={{ margin: 0 }}>RFID相控阵系统管理平台 v3.6</h2>
          <Space>
            <Text type="secondary">用户: {username}</Text>
            <Button type="text" icon={<LogoutOutlined />} onClick={onLogout}>
              退出
            </Button>
          </Space>
        </Header>
        <Content style={{ margin: 16, padding: 24, background: '#fff', borderRadius: 8, overflow: 'auto' }}>
          <Routes>
            <Route path="/" element={<Navigate to="/dashboard" replace />} />
            <Route path="/dashboard" element={<Dashboard />} />
            <Route path="/devices" element={<DeviceManagement />} />
            <Route path="/beam" element={<BeamControl />} />
            <Route path="/inventory" element={<Inventory />} />
            <Route path="/trajectory" element={<Trajectory />} />
            <Route path="/ledger" element={<AssetLedgerPage />} />
            <Route path="/ledger-import" element={<LedgerImport />} />
            <Route path="/erp-sync" element={<ErpSync />} />
            <Route path="/diff-analysis" element={<DiffAnalysis />} />
            <Route path="/diff-orders" element={<DiffWorkOrders />} />
            <Route path="/spatial-truth" element={<SpatialTruth />} />
            <Route path="/spatial-benchmark" element={<SpatialBenchmark />} />
            <Route path="/spatial-map" element={<SpatialMap />} />
            <Route path="/coordinate-system" element={<CoordinateSystemPage />} />
            <Route path="/settings" element={<Settings />} />
          </Routes>
        </Content>
      </Layout>
    </Layout>
  );
}

export default function App() {
  const [authed, setAuthed] = useState(false);
  const [username, setUsername] = useState('');

  useEffect(() => {
    const token = localStorage.getItem('pa_token');
    const user = localStorage.getItem('pa_username');
    if (token && user) {
      setAuthed(true);
      setUsername(user);
    }
  }, []);

  const handleLogout = () => {
    localStorage.removeItem('pa_token');
    localStorage.removeItem('pa_username');
    setAuthed(false);
    setUsername('');
  };

  if (!authed) {
    return (
      <Login
        onLoginSuccess={(_token, user) => {
          setAuthed(true);
          setUsername(user);
        }}
      />
    );
  }

  return (
    <BrowserRouter>
      <AppLayout username={username} onLogout={handleLogout} />
    </BrowserRouter>
  );
}
