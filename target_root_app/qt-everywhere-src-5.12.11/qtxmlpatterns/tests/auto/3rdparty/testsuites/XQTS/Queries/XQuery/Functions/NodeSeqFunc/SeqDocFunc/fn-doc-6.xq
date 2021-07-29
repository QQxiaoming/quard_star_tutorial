(: Name: fn-doc-6 :)
(: Description: Evaluation of fn:doc used with different resourses and the "is" operator.:)

(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

fn:doc($input-context1) is fn:doc($input-context2)