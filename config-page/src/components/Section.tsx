import React from 'react';

export const Section: React.FC<{ title?: string; id?: string; children: React.ReactNode }> = ({ title, id, children }) => (
  <section className="halite-section" id={id}>
    {title && <h2 className="halite-section-title">{title}</h2>}
    <div className="halite-section-content">
      {children}
    </div>
  </section>
);
