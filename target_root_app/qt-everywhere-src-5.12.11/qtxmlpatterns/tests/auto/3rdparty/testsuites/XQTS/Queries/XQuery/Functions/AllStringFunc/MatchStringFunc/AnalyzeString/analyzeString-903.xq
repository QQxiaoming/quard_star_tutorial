xquery version "1.1";

(: analyze-string, error, pattern matches a zero-length string :)

analyze-string("abc", "a|b|c?")