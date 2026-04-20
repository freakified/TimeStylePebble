import React from 'react';
import { PebbleConfigProvider, useConfig } from './context/PebbleConfigContext';
import { SettingsPage } from './pages';

const App: React.FC = () => {
  return (
    <PebbleConfigProvider>
      <SettingsPage />
    </PebbleConfigProvider>
  );
};

export default App;
