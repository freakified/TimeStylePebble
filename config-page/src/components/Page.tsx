import React from 'react';
import { useConfig } from '../context/PebbleConfigContext';

export const Page: React.FC<{ title: string; children: React.ReactNode }> = ({ title, children }) => {
  const { resetToDefaults, save } = useConfig();

  const handleReset = () => {
    const confirmed = window.confirm(
      'Reset all settings to their defaults? This will not affect saved custom themes.'
    );

    if (confirmed) {
      resetToDefaults();
    }
  };

  return (
    <div className="halite-page">
      <header className="halite-header">
        <h1>{title}</h1>
        <div className="halite-header-actions">
          <button className="halite-reset-button" onClick={handleReset}>RESET</button>
          <button className="halite-save-button" onClick={save}>SAVE</button>
        </div>
      </header>
      <main className="halite-content">
        {children}
      </main>
    </div>
  );
};
