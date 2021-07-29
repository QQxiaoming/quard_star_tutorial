(: Name: Catalog003 :)
(: Description: test all error test cases have at least one expected error :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<error-no-expected-error>{$input-context//*:test-case[@scenario = "runtime-error" or @scenario = "parse-error"][fn:count(*:expected-error) = 0 and fn:count(*:output-file[@compare = "Inspect"]) = 0]/@name/string()}</error-no-expected-error>