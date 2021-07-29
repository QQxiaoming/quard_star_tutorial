(:*******************************************************:)
(: Test: K2-SeqExprCast-426                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cast a set of QName to xs:string.            :)
(:*******************************************************:)
<e>
{
    "START",
    string(QName("http://example.com", "prefix:localName")),
    string(QName("http://example.com", "localName")),
    string(QName("http://example.com", "localName")),
    string(QName("", "localName")),
    string(QName((), "localName")),
    "END"
}
    </e>