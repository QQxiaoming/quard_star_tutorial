(:*******************************************************:)
(: Test: K-EscapeHTMLURIFunc-4                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Combine fn:concat and fn:escape-html-uri.    :)
(:*******************************************************:)
escape-html-uri("http://www.example.com/00/Weather/CA/Los Angeles#ocean")
			eq "http://www.example.com/00/Weather/CA/Los Angeles#ocean"