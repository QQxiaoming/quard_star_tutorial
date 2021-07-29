(: Name: fn-doc-32 :)
(: Description: Open an XML document that has ISO-8859-1 as encoding. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

string(fn:doc($input-context)),
string(exactly-one(fn:doc($input-context)/*))
