(: Name: Literals067 :)
(: Description: Test the escaping of the ' (apostrophe) and " (quotation) characters as part of an XML attribute constructor. Notice that the &quot; (quote) characters need to be entitized in the attribute content for XML validity :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<test check='He said, "I don''t like it."' />