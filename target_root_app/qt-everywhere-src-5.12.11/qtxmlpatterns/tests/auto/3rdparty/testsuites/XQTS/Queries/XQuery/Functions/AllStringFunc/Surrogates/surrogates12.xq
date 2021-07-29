(: Name: surrogates12 :)
(: Description: matches() when non-BMP characters are present :)

matches("abc&#x1D157;def", "abc[&#x1D156;-&#x1D158;]def")