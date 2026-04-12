module.exports = [
  {
    "type": "heading",
    "defaultValue": "HalfLife Settings"
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
        "defaultValue": "Drinking duration preferences",
        "size": 3
      },
      {
        "type": "input",
        "messageKey": "DrinkTiming[0]",
        "label": "Short drink duration (minutes)",
        "defaultValue": 5,
        "attributes": {
          "type": "number",
          "min": 1,
          "max": 120,
          "step": 1
        }
      },
      {
        "type": "input",
        "messageKey": "DrinkTiming[1]",
        "label": "Medium drink duration (minutes)",
        "defaultValue": 15,
        "attributes": {
          "type": "number",
          "min": 1,
          "max": 120,
          "step": 1
        }
      },
      {
        "type": "input",
        "messageKey": "DrinkTiming[2]",
        "label": "Long drink duration (minutes)",
        "defaultValue": 30,
        "attributes": {
          "type": "number",
          "min": 1,
          "max": 120,
          "step": 1
        }
      },
    ]
  },
  {
    "type": "section",
    "capabilities": ["COLOR"],
    "items": [
      {
        "type": "heading",
        "defaultValue": "Color settings",
        "size": 3
      },
      {
        "type": "input",
        "messageKey": "PreferMin",
        "label": "Minimum prefered blood content (mg)",
        "defaultValue": 120,
        "attributes": {
          "type": "number",
          "min": 0,
          "max": 999,
          "step": 1
        }
      },
      {
        "type": "input",
        "messageKey": "PreferMax",
        "label": "Maximum prefered blood content (mg)",
        "defaultValue": 170,
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
        "defaultValue": "Advanced settings",
        "size": 3
      },
      {
        "type": "input",
        "messageKey": "CustomCaff",
        "label": "Custom drink default caffeine content (mg)",
        "defaultValue": 105,
        "attributes": {
          "type": "number",
          "min": 1,
          "max": 999,
          "step": 1
        }
      },
      {
        "type": "input",
        "messageKey": "CustomStep",
        "label": "Custom drink adjustment step (mg)",
        "defaultValue": 5,
        "attributes": {
          "type": "number",
          "min": 1,
          "max": 100,
          "step": 1
        }
      },
      {
        "type": "text",
        "defaultValue": "Do not set half-lifes too close, maintain at least 10min difference, otherwise simulation errors might become very large.",
      },
      {
        "type": "input",
        "messageKey": "AbsorbtionHL",
        "label": "Absorbtion (gut to blood) half-life (minutes)",
        "defaultValue": 20,
        "attributes": {
          "type": "number",
          "min": 5,
          "max": 120,
          "step": 1
        }
      },
      {
        "type": "input",
        "messageKey": "EliminationHL",
        "label": "Elimination (decay) half-life (minutes)",
        "defaultValue": 300,
        "attributes": {
          "type": "number",
          "min": 90,
          "max": 1600,
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
