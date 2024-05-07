import civetPlugin from 'eslint-plugin-civet/ts';

export default [
    ...civetPlugin.configs.jsRecommended,
    ...civetPlugin.configs.strict,
    {
        languageOptions: {
            ecmaVersion: 2022,
            sourceType: 'script'
        },
        rules: {
            '@typescript-eslint/ban-ts-comment': ['error', { 'ts-nocheck': false }],
            '@typescript-eslint/consistent-type-imports': 'error',
            '@typescript-eslint/default-param-last': 'error',
            '@typescript-eslint/no-non-null-assertion': 'off',
            '@typescript-eslint/no-unused-vars': 'off',
            // https://github.com/eslint/eslint/issues/15896
            'no-var': 'off',
            'no-restricted-syntax': ['error', {
                selector: "VariableDeclaration[kind='var'][declare!=true]",
                message: 'Unexpected var, use let or const instead.'
            }]
        }
    }
];
