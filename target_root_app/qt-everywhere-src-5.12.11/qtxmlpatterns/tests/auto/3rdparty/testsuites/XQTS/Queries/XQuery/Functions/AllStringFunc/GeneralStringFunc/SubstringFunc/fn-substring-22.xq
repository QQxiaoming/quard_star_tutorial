(: Name: fn-substring-22:)
(: Description: Evaluation of substring function, where start<0 and (start+len)<0 :)

let $var := document{<concepts id="5555 6666"/>} return
concat('#', fn:substring($var/concepts/@id, string-length($var/concepts/@id) - 18, 1), '#')