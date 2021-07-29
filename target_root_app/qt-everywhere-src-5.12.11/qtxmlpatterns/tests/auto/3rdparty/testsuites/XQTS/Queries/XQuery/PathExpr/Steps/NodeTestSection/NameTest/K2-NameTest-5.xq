(:*******************************************************:)
(: Test: K2-NameTest-5                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A complex expression to parse, taken from W3C's (obsolete) 'Building a Tokenizer for XPath or XQuery' document. The query naturally contains XPTY0004. XPDY0002 is allowed since an implementation may change the default focus from being 'none' to being undefined. :)
(:*******************************************************:)
declare namespace namespace = "http://example.com";
declare union <union>for gibberish {
   for $for in for return <for>***div div</for>
}</union>,
if(if) then then else else- +-++-**-* instance
of element(*)* * * **---++div- div -div
