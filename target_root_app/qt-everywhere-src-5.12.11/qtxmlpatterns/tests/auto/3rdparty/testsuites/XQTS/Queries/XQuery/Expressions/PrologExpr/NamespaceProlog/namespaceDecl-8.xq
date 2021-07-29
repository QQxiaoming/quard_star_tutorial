(: Name: namespaceDecl-8:)
(: Description: Evaluates that at a namespace delcaration the prefix name is an NCName. :)
(: from the query specs. :)

declare namespace foo:bar = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aa"