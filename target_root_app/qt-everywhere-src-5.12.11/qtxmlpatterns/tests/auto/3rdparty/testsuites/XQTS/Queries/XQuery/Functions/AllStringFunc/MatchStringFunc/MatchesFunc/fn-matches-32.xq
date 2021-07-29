xquery version "1.1";

(: Name: fn-matches-32 :)
(: Description: Evaluation of matches function with "q" flag (allowed in XQuery 1.1 :)


fn:matches("abracadabra", "(?:abra(?:cad)?)*", "q")