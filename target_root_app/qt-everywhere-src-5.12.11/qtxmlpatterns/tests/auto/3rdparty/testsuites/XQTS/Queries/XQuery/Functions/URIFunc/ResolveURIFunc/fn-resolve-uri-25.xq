(: Name: fn-resolve-uri-24 :)
(: Description: Evaluation of resolve-uri function with an absolute URI for the first argument and 
   a relative URI reference for second argument.:)
(: Should return first argument unchanged :)

string(resolve-uri("http://www.example.com/a.html","b.html"))
