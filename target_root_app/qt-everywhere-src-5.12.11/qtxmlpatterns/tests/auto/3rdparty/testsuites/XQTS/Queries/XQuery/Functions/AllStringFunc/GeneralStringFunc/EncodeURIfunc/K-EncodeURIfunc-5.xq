(:*******************************************************:)
(: Test: K-EncodeURIfunc-5                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Combine fn:concat and fn:encode-for-uri.     :)
(:*******************************************************:)
concat("http://www.example.com/", encode-for-uri("100% organic")) 
			eq "http://www.example.com/100%25%20organic"