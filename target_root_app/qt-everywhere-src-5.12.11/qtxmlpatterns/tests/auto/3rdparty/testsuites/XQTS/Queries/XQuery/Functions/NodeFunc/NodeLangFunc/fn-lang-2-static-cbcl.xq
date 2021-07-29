(: Name: fn-lang-2:)
(: Description: Evaluation of the fn:lang function with testlang set to empty sequence:)
(: Uses fn:count to avoid empty file.    :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:count(fn:lang((), exactly-one($input-context1/langs[1]/para[1])))
