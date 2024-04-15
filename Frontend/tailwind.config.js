/** @type {import('tailwindcss').Config} */
module.exports = {
  content: [
    "./src/**/*.{js,jsx,ts,tsx}"
  ],
  theme: {
    extend: {backgroundColor: theme => ({
      ...theme('colors'),
      'app-background': 'var(--app-background-color)',
    }),
    screens: {
      'smm': '450px',
      },
    },
  },
  plugins: [],
}
