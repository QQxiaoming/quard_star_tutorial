(:*******************************************************:)
(: Test: K2-DirectConElemWhitespace-9                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: No escaping mechanism applies for quotes/apostrophes inside element content. :)
(:*******************************************************:)
string(<e>'a''a'''a"a""a"""a"</e>) 
	eq "'a''a'''a""a""""a""""""a"""