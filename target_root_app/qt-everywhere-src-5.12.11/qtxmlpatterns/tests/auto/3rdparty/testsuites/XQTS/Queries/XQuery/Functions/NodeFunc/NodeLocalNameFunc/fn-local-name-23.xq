(: Name: fn-local-name-23:)
(: Description: Evaluation of the fn:local-name function with second argument set to "." and no context node set.:)
(: Use fn:string.  :)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
    fn:local-name(.)
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()