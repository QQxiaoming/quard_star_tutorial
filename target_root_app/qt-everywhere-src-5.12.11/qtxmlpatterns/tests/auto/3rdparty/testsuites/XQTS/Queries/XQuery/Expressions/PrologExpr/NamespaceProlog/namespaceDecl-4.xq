(: Name: namespaceDecl-4:)
(: Description: Evaluation of redefinition of namespace associated with xml.:)

declare namespace foo = "http://www.w3.org/XML/1998/namespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"a"