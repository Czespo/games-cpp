A Divergence level is written using the following rules:

- `.` ( period/decimal, a goal, the positions where boxes need to be placed. )
- `$` ( dollar, a box. )
- `*` ( asterisk, a box on top of a goal. )
- `@` ( at symbol, the player. )
- `&` ( ampersand, the player on top of a goal. )
- `#` ( hash, a wall. )
- ` ` ( space, an empty floor. )

Additionally, a new line is used to start a new row of a Divergence level.

The Divergence level file ( "levels" ) is written in comma-delimited
format, with a terminal comma indicating the end of the last level.
The commas must be on a line of their own.
```
LEVEL
,
LEVEL
,
LEVEL
,
```
See "levels" for this format in action.
