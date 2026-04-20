export const copyToClipboard = async (text: string): Promise<boolean> => {
    // 1. Try modern async clipboard API
    if (navigator.clipboard && navigator.clipboard.writeText) {
        try {
            await navigator.clipboard.writeText(text);
            return true;
        } catch (err) {
            console.warn('Modern clipboard API failed:', err);
        }
    }

    // 2. Fallback to execCommand (iOS/WKWebView compatible)
    return new Promise((resolve) => {
        try {
            const textArea = document.createElement("textarea");
            textArea.value = text;

            // Avoid scrolling to bottom
            textArea.style.position = "fixed";
            textArea.style.top = "0";
            textArea.style.left = "0";
            textArea.style.opacity = "0";
            textArea.style.pointerEvents = "none";

            document.body.appendChild(textArea);

            // iOS Safari requires focus before selection
            textArea.focus();
            textArea.select();

            // Explicit selection for iOS
            try {
                textArea.setSelectionRange(0, 999999);
            } catch (e) {
                // Ignore, may throw on some browsers
            }

            const successful = document.execCommand('copy');
            document.body.removeChild(textArea);
            resolve(successful);
        } catch (err) {
            console.warn('execCommand fallback failed:', err);
            resolve(false);
        }
    });
};
