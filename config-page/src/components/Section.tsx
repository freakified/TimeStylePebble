import React from 'react';

export const Section: React.FC<{ title?: string; children: React.ReactNode }> = ({ title, children }) => (
  <section className="halite-section">
    {title && <h2 className="halite-section-title">{title}</h2>}
    <div className="halite-section-content">
      {children}
    </div>
  </section>
);
