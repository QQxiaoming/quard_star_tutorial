(: Name: fn-lang-1:)
(: Description: Evaluation of the fn:lang function with no second argument and no context node.:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:lang("en")
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()