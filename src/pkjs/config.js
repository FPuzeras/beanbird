module.exports = [
  {
    "type": "heading",
    "defaultValue": "BeanBird Settings"
  },
  {
    "type": "text",
    "defaultValue": "Customize your caffeine preferences."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Top group drinks",
        "size": 3
      },
      {
        "type": "section",
        "items": [
          {
            "type": "heading",
            "defaultValue": "Top drink",
            "size": 5
          },
          {
            "type": "input",
            "messageKey": "DrinkTitle[0]",
            "label": "Drink title",
            "defaultValue": "Tea",
            "attributes": {
              "type": "text",
              "maxlength": 15 // from STR_MAXLEN in "src/c/settings.h"   
            }
          },
          {
            "type": "input",
            "messageKey": "CaffContent[0]",
            "label": "Caffeine content (mg)",
            "defaultValue": 50,
            "attributes": {
              "type": "number",
              "min": 0,
              "max": 999,
              "step": 1
            }
          }
        ]
      },
      {
        "type": "section",
        "items": [
          {
            "type": "heading",
            "defaultValue": "Middle drink",
            "size": 5
          },
          {
            "type": "input",
            "messageKey": "DrinkTitle[1]",
            "label": "Drink title",
            "defaultValue": "Coffee (M)",
            "attributes": {
              "type": "text",
              "maxlength": 15 // from STR_MAXLEN in "src/c/settings.h"   
            }
          },
          {
            "type": "input",
            "messageKey": "CaffContent[1]",
            "label": "Caffeine content (mg)",
            "defaultValue": 90,
            "attributes": {
              "type": "number",
              "min": 0,
              "max": 999,
              "step": 1
            }
          }
        ]
      },
      {
        "type": "section",
        "items": [
          {
            "type": "heading",
            "defaultValue": "Bottom drink",
            "size": 5
          },
          {
            "type": "input",
            "messageKey": "DrinkTitle[2]",
            "label": "Drink title",
            "defaultValue": "Coffee (L)",
            "attributes": {
              "type": "text",
              "maxlength": 15 // from STR_MAXLEN in "src/c/settings.h"   
            }
          },
          {
            "type": "input",
            "messageKey": "CaffContent[2]",
            "label": "Caffeine content (mg)",
            "defaultValue": 150,
            "attributes": {
              "type": "number",
              "min": 0,
              "max": 999,
              "step": 1
            }
          }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Bottom group drinks",
        "size": 3
      },
      {
        "type": "section",
        "items": [
          {
            "type": "heading",
            "defaultValue": "Top drink",
            "size": 5
          },
          {
            "type": "input",
            "messageKey": "DrinkTitle[3]",
            "label": "Drink title",
            "defaultValue": "RedBull 250ml",
            "attributes": {
              "type": "text",
              "maxlength": 15 // from STR_MAXLEN in "src/c/settings.h"   
            }
          },
          {
            "type": "input",
            "messageKey": "CaffContent[3]",
            "label": "Caffeine content (mg)",
            "defaultValue": 80,
            "attributes": {
              "type": "number",
              "min": 0,
              "max": 999,
              "step": 1
            }
          }
        ]
      },
      {
        "type": "section",
        "items": [
          {
            "type": "heading",
            "defaultValue": "Middle drink",
            "size": 5
          },
          {
            "type": "input",
            "messageKey": "DrinkTitle[4]",
            "label": "Drink title",
            "defaultValue": "RedBull 355ml",
            "attributes": {
              "type": "text",
              "maxlength": 15 // from STR_MAXLEN in "src/c/settings.h"   
            }
          },
          {
            "type": "input",
            "messageKey": "CaffContent[4]",
            "label": "Caffeine content (mg)",
            "defaultValue": 113,
            "attributes": {
              "type": "number",
              "min": 0,
              "max": 999,
              "step": 1
            }
          }
        ]
      },
      {
        "type": "section",
        "items": [
          {
            "type": "heading",
            "defaultValue": "Bottom drink",
            "size": 5
          },
          {
            "type": "input",
            "messageKey": "DrinkTitle[5]",
            "label": "Drink title",
            "defaultValue": "RedBull 500ml",
            "attributes": {
              "type": "text",
              "maxlength": 15 // from STR_MAXLEN in "src/c/settings.h"   
            }
          },
          {
            "type": "input",
            "messageKey": "CaffContent[5]",
            "label": "Caffeine content (mg)",
            "defaultValue": 160,
            "attributes": {
              "type": "number",
              "min": 0,
              "max": 999,
              "step": 1
            }
          }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Custom drink settings",
        "size": 3
      },
      {
        "type": "input",
        "messageKey": "CustomDefault",
        "label": "Custom drink default caffeine content (mg)",
        "defaultValue": 100,
        "attributes": {
          "type": "number",
          "min": 0,
          "max": 999,
          "step": 1
        }
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
