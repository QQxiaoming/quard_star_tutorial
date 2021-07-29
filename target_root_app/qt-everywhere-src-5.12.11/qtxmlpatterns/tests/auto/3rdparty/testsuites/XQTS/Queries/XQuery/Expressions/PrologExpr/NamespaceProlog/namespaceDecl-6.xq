(: Name: namespaceDecl-6:)
(: Description: Evaluation of typical usage of namespace declaration as per example 1 in section 4.7 of the query specs.:)

declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<foo:bar> Lentils </foo:bar>
