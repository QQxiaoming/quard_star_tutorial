(: Name: fn-resolve-uri-24 :)
(: Description: Evaluation of resolve-uri function with a relative URI reference for second argument.:)
(: Expects error FORG0002: see erratum FO.E1 :)


fn:string(fn:resolve-uri("a.html","b.html"))
