import React, { useState } from 'react';
import { Modal, ModalOverlay, Dialog, Button, Heading } from 'react-aria-components';

interface ImportThemeModalProps {
    isOpen: boolean;
    onOpenChange: (isOpen: boolean) => void;
    onImport: (json: string) => { success: boolean; error?: string };
}

export const ImportThemeModal: React.FC<ImportThemeModalProps> = ({
    isOpen,
    onOpenChange,
    onImport,
}) => {
    const [jsonText, setJsonText] = useState('');
    const [error, setError] = useState('');

    const handleImport = () => {
        const result = onImport(jsonText);
        if (result.success) {
            setJsonText('');
            setError('');
            onOpenChange(false);
        } else {
            setError(result.error || 'Import failed');
        }
    };

    const handleOpenChange = (open: boolean) => {
        if (!open) {
            setJsonText('');
            setError('');
        }
        onOpenChange(open);
    };

    return (
        <ModalOverlay
            isOpen={isOpen}
            onOpenChange={handleOpenChange}
            className="halite-color-modal-overlay"
            isDismissable
        >
            <Modal className="halite-color-modal halite-import-modal">
                <Dialog className="halite-color-dialog">
                    {({ close }) => (
                        <>
                            <div className="halite-color-modal-header">
                                <Heading slot="title">Import Theme</Heading>
                                <Button className="halite-color-modal-close" onPress={close}>
                                    ×
                                </Button>
                            </div>
                            <div className="halite-import-content">
                                <textarea
                                    className="halite-import-textarea"
                                    placeholder="Paste theme JSON here…"
                                    value={jsonText}
                                    onChange={(e) => {
                                        setJsonText(e.target.value);
                                        setError('');
                                    }}
                                    rows={8}
                                />
                                {error && <p className="halite-import-error">{error}</p>}
                                <Button
                                    className="halite-import-button"
                                    onPress={handleImport}
                                    isDisabled={!jsonText.trim()}
                                >
                                    Import
                                </Button>
                            </div>
                        </>
                    )}
                </Dialog>
            </Modal>
        </ModalOverlay>
    );
};
