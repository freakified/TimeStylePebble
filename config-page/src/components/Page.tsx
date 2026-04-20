import React from 'react';
import { useConfig } from '../context/PebbleConfigContext';

export const Page: React.FC<{ title: string; children: React.ReactNode }> = ({ title, children }) => {
  const { save } = useConfig();

  return (
    <div className="halite-page">
      <header className="halite-header">
        <h1>{title}</h1>
        <button className="halite-save-button" onClick={save}>SAVE</button>
      </header>
      <main className="halite-content">
        {children}
      </main>
    </div>
  );
};
