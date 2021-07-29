(:Test: fn-compare-22                                     :)
(:Description Evaluates The "compare" function with a nonexistent collation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:compare("a","a","CollationA")