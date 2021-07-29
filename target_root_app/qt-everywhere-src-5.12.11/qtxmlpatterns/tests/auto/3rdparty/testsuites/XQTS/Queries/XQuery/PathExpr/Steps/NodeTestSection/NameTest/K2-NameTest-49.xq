(:*******************************************************:)
(: Test: K2-NameTest-49                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An child axis applies on a sequence of attributes. :)
(:*******************************************************:)
declare namespace p = "http://example.com/";
		    <a>{<e p:a="1" p:b="2" p:c="3"/>/attribute::*/p:*}</a>