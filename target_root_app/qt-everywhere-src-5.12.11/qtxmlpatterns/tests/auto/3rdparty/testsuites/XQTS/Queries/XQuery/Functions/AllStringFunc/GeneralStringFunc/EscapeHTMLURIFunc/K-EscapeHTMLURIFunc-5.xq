(:*******************************************************:)
(: Test: K-EscapeHTMLURIFunc-5                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Combine fn:concat and fn:escape-html-uri.    :)
(:*******************************************************:)
escape-html-uri("javascript:if (navigator.browserLanguage == 'fr') window.open('http://www.example.com/~bébé');")
			eq "javascript:if (navigator.browserLanguage == 'fr') window.open('http://www.example.com/~b%C3%A9b%C3%A9');"