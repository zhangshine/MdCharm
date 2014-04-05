Markdown(Extra) Cheat Sheet
====================
* [Blockquotes\(Ctrl+Q\)](#blockquotes)
* [Code Block](#codeBlock)
* [Emphasis](#emphasis)
* [Headers](#headers)
* [Horizontal rules](#horizontalRules)
* [Images\(Ctrl+Shitf+L\)](#images)
* [Inline Html](#inlineHtml)
* [Links\(Ctrl+L\)](#links)
* [Lists](#lists)
* [Paragraph](#paragraph)
* [Table](#table)
* [Literal Characters](#literal)
* [Footnote](#footnote)
* [Math](#math)
* [Multi-Markdown](#mmd)

Blockquotes {#blockquotes}
--------------------------
ShortCuts: `Ctrl+Q`  
Markdown uses email-style `>` characters for blockquoting.  
`> This is a blockquote`
******************************************************************************
Code Block {#codeBlock}
-----------------------
Indent 4 spaces or 1 tab
```markdown
This is a normal paragraph:

	This is a code block.
```
**or (code syntax highlight)** 

	```javascript
	for(var i=0; i<100; i++)
		console.log("Hello, MdCharm");
	```
Result:
```javascript
for(var i=0; i<100; i++)
	console.log("Hello, MdCharm");
```
**or**

	~~~~~~~~~~~~~~~~~~~~~
	Here is the code
	~~~~~~~~~~~~~~~~~~~~~
******************************************************************************
Emphasis {#emphasis}
--------------------
       | Shortcuts | Syntax              | Result
-------|-----------|---------------------|-------
Italic | Ctrl+I    | \*italic\* **or** \_italis\_ | *italic*
Bold   | Ctrl+B    | \*\*bold\*\* **or** \_\_bold\_\_ | **bold**
Strike through | Ctrl+T | \~~Strike through~~ | ~~Strike through~~
******************************************************************************
Headers {#headers}
------------------
```markdown
This is an H1
=============

This is an H2
-------------
```
*or*
```markdown
# This is an H1

## This is an H2

###### This is an H6
```
**Header Id Attribute**
```markdown
## Header 2 ## {#header2}
```
*or*
```markdown
Header 1 {#header1}
========
```
**Link back to header**
```markdown
[Link back to header 1](#header1)
```
******************************************************************************
Horizontal rules {#horizontalRules}
-----------------------------------
```markdown
* * *

***

*****
	
- - -

---------------------------------------

_ _ _
```
******************************************************************************
Images {#images}
----------------
Markdown Input | Result
---------------|-------
`![MdCharm](qrc:/mdcharm.png "MdCharm")` | ![MdCharm](qrc:/mdcharm.png "MdCharm")

**width and height attribute**  
*Please [enable Multi-Markdown](#mmd "") first.*   
```
![MdCharmWH][]

[MdCharmWH]: qrc:/mdcharm.png width=100px height=100px
```
**result**
<figure>
<img src="qrc:/mdcharm.png" alt="MdCharmWH" id="mdcharmwh" style="height:100px;width:100px;" />
<figcaption>MdCharmWH</figcaption></figure>

******************************************************************************
Inline Html {#inlineHtml}
-------------------------
```markdown
This is a regular paragraph.

<table>
    <tr>
        <td>Foo</td>
    </tr>
</table>

This is another regular paragraph.
```
******************************************************************************
Links {#links}
--------------
Markdown Input                                                           | Result
-------------------------------------------------------------------------|------------------------------------------------------------------------
`This is [an example](http://example.com/ "Optional Title") inline link.`| This is [an example](http://example.com/ "Optional Title") inline link.
`<http://example.com/>`                                                  | <http://example.com/>
******************************************************************************
Lists {#lists}
--------------
**Unordered lists**  
Asterisks, plus signs or dashes:
```markdown
* Red
* Green
* Blue

+ Red
+ Green
+ Blue

- Red
- Green
- Blue
```
**Ordered lists**
```markdown
1. One
2. Two
3. Three
```
******************************************************************************
Paragraph {#paragraph}
----------------------
```markdown
One or more consecutive lines of text
separated by one or more blank lines.
 
This is another paragraph.
```
**Line Break**  
To create a line break, end a line in a paragraph with two or more spaces
******************************************************************************
Table {#table}
--------------
```markdown
First Header  | Second Header
--------------|--------------
Content Cell  | Content Cell
Content Cell  | Content Cell
```
or
```markdown
|First Header  | Second Header|
|--------------|--------------|
|Content Cell  | Content Cell |
|Content Cell  | Content Cell |
```
Left aligned
```markdown
| Item     | Value |
|----------|:------|
| Computer | $16   |
```
Right aligned
```markdown
| Item     | Value |
|----------|------:|
| Computer | $16   |
```
Center aligned
```markdown
| Item     | Value |
|----------|:-----:|
| Computer | $16   |
```
******************************************************************************
Literal Characters {#literal}
------------------
from <http://daringfireball.net/projects/markdown/syntax#backslash>
> Markdown allows you to use backslash escapes to generate literal
> characters which would otherwise have special meaning in Markdown's
> formatting syntax. For example, if you wanted to surround a word with
> literal asterisks (instead of an HTML `<em>` tag), you can backslashes
> before the asterisks, like this:
> 
>     \*literal asterisks\*
> 
> Markdown provides backslash escapes for the following characters:
> 
>     \   backslash
>     `   backtick
>     *   asterisk
>     _   underscore
>     {}  curly braces
>     []  square brackets
>     ()  parentheses
>     #   hash mark
> 	  +	  plus sign
> 	  -	  minus sign (hyphen)
>     .   dot
>     !   exclamation mark

Footnote[^footnote] {#footnote}
--------
```
Footnote example[^footnote].

[^footnote]: I am a footnote.
```
[^footnote]: I am a footnote

Math {#math}
----
**Please [enable Multi-Markdown](#mmd "") first.**   
*Export to PDF, ODT and Live Preview are not supported*   
form: http://fletcher.github.com/peg-multimarkdown/#math
> ```
> HTML Header: <script type="text/javascript"
> src="http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML">
> </script>
> 
> An example of math within a paragraph --- \\({e}^{i\pi }+1=0\\)
> --- easy enough.
> 
> And an equation on it's own:
> 
> \\[ {x}_{1,2}=\frac{-b\pm \sqrt{{b}^{2}-4ac}}{2a} \\]
> 
> That's it.
```

Multi-Markdown {#mmd}
--------------
**Enable Multi-Markdown:** Select `Settings` -> `Preference...` -> `Environment`, change `Markdown Engine` to `MultiMarkdown`