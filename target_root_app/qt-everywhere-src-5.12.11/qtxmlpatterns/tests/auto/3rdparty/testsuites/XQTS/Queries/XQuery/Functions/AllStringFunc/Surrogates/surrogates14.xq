(: Name: surrogates14 :)
(: Description: replace() when non-BMP characters are present :)

replace("abc&#119130;def", "[&#119120;-&#119135;]", "&#119135;")