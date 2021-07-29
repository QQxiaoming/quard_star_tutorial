xquery version "1.1";

(: test typing of result of analyze-string: with no import-schema :)

let $result := analyze-string("banana", "(b)(anana)")
return ($result//@nr)[1] instance of attribute(nr, xs:positiveInteger)