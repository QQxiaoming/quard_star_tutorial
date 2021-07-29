(: Name: base-URI-8 :)
(: Description: Evaluates base-uri with the fn:resolve-uri function.  Base URI not initialized:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   if (fn:static-base-uri() eq  fn:resolve-uri("examples")) then
    "true"
  else
    "true"
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()