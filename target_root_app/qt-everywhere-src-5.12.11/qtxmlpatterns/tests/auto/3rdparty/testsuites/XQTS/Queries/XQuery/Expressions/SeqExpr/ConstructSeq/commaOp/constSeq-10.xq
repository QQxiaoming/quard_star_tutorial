(: Name: constSeq-10:)
(: Description: Constructing sequences.  Constructing a sequence, where both members contains invocation to "fn:not()" function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:not("true"),fn:not("false"))