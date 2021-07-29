xquery version "1.1";

(: analyze-string, with i and x flag :)

analyze-string("how now brown cow", " (HOW) | (NOW) &#xa;|
                                      (BROWN) | (COW) ", "ix")