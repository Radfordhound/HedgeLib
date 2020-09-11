# Translator's Guide

This file explains how to translate the text used by the HedgeTools to another language.

Firstly, **thank you so much** for taking the time to do this!!

I tried to make this process as simple as possible with the intent of not wasting your time. If you encounter any issues while following this guide, please let me know by [creating an issue](https://github.com/Radfordhound/HedgeLib/issues/new).

## How to add a language

Each HedgeTool has two files under its "src" directory related to text:

- languages.h
- text.h

**languages.h** is simply a list of every language which is supported by the tool, like this:
   
```c
LANGUAGE(ENGLISH)
LANGUAGE(SPANISH)
```

etc.

To add a new language, simply add a new ```LANGUAGE(WHATEVER)``` entry to the end of "languages.h".

**text.h** is a little more complicated. It contains one list of every "string" of text used
by the tool for each language, like this:

```c
LOCALIZED_TEXT(ENGLISH)
{
    /* PRESS_ENTER_STRING */
    HL_NTEXT("Press enter to continue..."),

    /* ERROR_UNKNOWN_STRING */
    HL_NTEXT("ERROR: An unknown error was encountered.")
};

LOCALIZED_TEXT(SPANISH)
{
    /* PRESS_ENTER_STRING */
    HL_NTEXT("Presione enter para continuar ..."),

    /* ERROR_UNKNOWN_STRING */
    HL_NTEXT("ERROR: No sé español, y Google Translate no es genial. :(")
};
```

etc.

To add localized text for a new language, simply copy the English ```LOCALIZED_TEXT``` list at the top of the file, paste it at the bottom, and, of course, change the language ID (the part that says "ENGLISH") and strings of text.

That's all!
...Well, almost.

## Some notes on C Formatting
Both of these .h files are actually C code, so there's a few rules you have to follow to prevent code errors:

- **The quotation marks (```"```) surrounding each "string" of text are very important**; they tell C that these characters are a string of text! Without them, C gets very confused. Make sure to include them!
- **If you see a percent sign followed by a character like ```%s```, ```%v```, etc. please include it somewhere in your translation!** That's a part of the string that's going to be replaced with a value.

  For example, in this string:

  ```c
  "Format: NN for %s Version %u"
  ```

  The ```%s``` and ```%u``` will be replaced with different values elsewhere in the tool's code. For example:
  ```
  Format: NN for PlayStation2 Version 1
  ```
  ```
  Format: NN for Gamecube Version 1
  ```
  ```
  Format: NN for Xbox Version 2
  ```

  etc.

- **The next "string" of text doesn't actually begin until the comma (```,```) at the end**, so you can do some neat things, like this:
  ```c
  LOCALIZED_TEXT(ENGLISH)
  {
      /* LONG_STRING */
      HL_NTEXT("This is an example of how you can format a long string ")
      HL_NTEXT("without making the code look ugly. Notice how this string is ")
      HL_NTEXT("still going despite being split up into multiple lines?"),

      /* ANOTHER_STRING */
      HL_NTEXT("This is a new string now thanks to that comma.")
  };
  ```
  This will result in just two strings:
  - ```This is an example of how you can format a long string without making the code look ugly. Notice how this string is still going despite being split up into multiple lines?```
  - ```This is a new string now thanks to that comma.```

- **If you want to put a quotation mark (```"```) character in your string, type this:** ```\"``` C will replace that with a single quotation mark.
- **If you want to put a newline in your string, type this:** ```\n``` C will replace that with a newline, as if you actually hit enter/return on your keyboard.
- **If you want to put a tab character in your string, type this:** ```\t``` C will replace that with a tab character, as if you actually hit tab on your keyboard.
- **If you want a normal backslash, just type two of them:** ```\\``` C will replace the two backslashes with just one backslash. As you may have guessed from the above points, the ```\``` character is actually a special character in C that means "replace this with something else"; it's not a normal backslash, hence why you have to type two of them.

That's all! If you have any issues following this guide, please [create an issue](https://github.com/Radfordhound/HedgeLib/issues/new) describing your problem. Thanks once again!
