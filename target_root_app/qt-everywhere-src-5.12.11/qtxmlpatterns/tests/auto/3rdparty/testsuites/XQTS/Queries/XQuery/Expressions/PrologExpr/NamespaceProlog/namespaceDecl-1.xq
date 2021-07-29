(: Name: namespaceDecl-1:)
(: Description: Evaluation of multiple namespace declarations with same prefix.  Should raise static error.:)

declare namespace foo = "http://www.example.com/examples";
declare namespace foo = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

a