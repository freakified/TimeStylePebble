import React from 'react';

const DONATION_URL = 'https://ko-fi.com/freakified';
const DISMISSED_STORAGE_KEY = 'timestyle-donation-dismissed';

function loadDismissedState(): boolean {
  try {
    return localStorage.getItem(DISMISSED_STORAGE_KEY) === 'true';
  } catch {
    return false;
  }
}

function saveDismissedState() {
  try {
    localStorage.setItem(DISMISSED_STORAGE_KEY, 'true');
  } catch {
    // Ignore storage failures; dismissing still works for this session.
  }
}

export const DonationLink: React.FC = () => {
  const [isDismissed, setIsDismissed] = React.useState(loadDismissedState);

  const handleDismiss = () => {
    saveDismissedState();
    setIsDismissed(true);
  };

  if (isDismissed) {
    return null;
  }

  return (
    <aside className="halite-donation" aria-labelledby="halite-donation-title">
      <div className="halite-donation-copy">
        <h2 id="halite-donation-title">Created by Freakified</h2>
        <p>Like TimeStyle? Please consider supporting its development!</p>
      </div>
      <div className="halite-donation-actions">
        <a className="halite-donation-button" href={DONATION_URL} target="_blank" rel="noreferrer">
          Donate
        </a>
        <button className="halite-donation-dismiss" type="button" onClick={handleDismiss}>
          Never show again
        </button>
      </div>
    </aside>
  );
};
