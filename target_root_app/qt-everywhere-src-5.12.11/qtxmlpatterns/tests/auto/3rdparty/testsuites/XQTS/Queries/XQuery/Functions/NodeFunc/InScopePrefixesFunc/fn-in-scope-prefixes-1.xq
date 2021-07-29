(: Name: fn-in-scope-prefixes-1 :)
(: Description: Evaluation of fn:in-scope-prefixes function with incorrect arity.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:in-scope-prefixes(<a1 xmlns:p1="http://www.exampole.com"></a1>,"Second Argument")