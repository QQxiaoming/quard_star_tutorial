(:*******************************************************:)
(: Test: K2-NameTest-50                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A '@prefix:*'-test doesn't match element nodes. :)
(:*******************************************************:)
declare namespace p = "http://example.com/";
		    <a>{document {<p:e/>}/@p:*}</a>