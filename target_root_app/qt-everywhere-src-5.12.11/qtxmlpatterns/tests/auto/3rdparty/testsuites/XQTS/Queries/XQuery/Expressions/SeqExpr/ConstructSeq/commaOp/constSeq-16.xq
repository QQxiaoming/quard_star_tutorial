(: Name: constSeq-16:)
(: Description: Constructing sequences.  Constructing a sequence, where both of its members contains invocation to "xs:anyURI".:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:anyURI("http://www.example.com"),xs:anyURI("http://www.example1.com"))