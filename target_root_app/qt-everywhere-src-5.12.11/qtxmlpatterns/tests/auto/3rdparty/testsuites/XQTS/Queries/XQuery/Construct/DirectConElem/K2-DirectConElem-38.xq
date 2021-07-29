(:*******************************************************:)
(: Test: K2-DirectConElem-38                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Bind the 'xml' namespace URI to a valid prefix. The output doesn't expect the declaration because the c14n specification ignores declarations of the xml prefix if it binds to the XML namespace(see section 2.3). In either case, serializing this declaration is redundant. See the public report #4217 in W3C's Bugzilla database. :)
(:*******************************************************:)
<e xmlns:xml="http://www.w3.org/XML/1998/namespace"/>