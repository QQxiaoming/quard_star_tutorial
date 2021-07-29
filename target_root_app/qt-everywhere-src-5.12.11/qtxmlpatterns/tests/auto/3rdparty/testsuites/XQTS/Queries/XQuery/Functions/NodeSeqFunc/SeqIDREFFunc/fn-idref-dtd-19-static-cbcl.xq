(: Name: fn-idref-dtd-19 :)
(: Description: Evaluation of fn:idref for which $arg uses the fn:concat function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{fn:idref(fn:concat("i","d1"), exactly-one($input-context1/IDS[1]))}</results>
