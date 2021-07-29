xquery version "1.1";

(: test typing of result of analyze-string: with no import-schema :)

let $result := analyze-string("banana", "(b)(anana)")
return $result instance of element(*, xs:untyped)