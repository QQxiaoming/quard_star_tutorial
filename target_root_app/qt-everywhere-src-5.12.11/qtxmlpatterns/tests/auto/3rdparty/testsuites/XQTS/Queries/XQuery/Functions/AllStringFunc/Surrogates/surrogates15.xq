(: Name: surrogates15 :)
(: Description: replace() when non-BMP characters are present :)

replace("abc&#x1D157;def", "[^a-f]", "###")