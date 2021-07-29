(:Test: fn-implicit-timezone-1                           :)
(:Description Evaluation of "fn:implicit-timezone" with   :)
(:incorrect arity.                                       :)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:implicit-timezone("Argument 1")