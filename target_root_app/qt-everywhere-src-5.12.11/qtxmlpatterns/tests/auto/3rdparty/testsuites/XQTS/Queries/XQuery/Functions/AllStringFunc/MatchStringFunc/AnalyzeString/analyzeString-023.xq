xquery version "1.1";

(: test string value of result of analyze-string :)

let $result := analyze-string("banana", "(b)(anana)")
return string($result/fn:match[1])