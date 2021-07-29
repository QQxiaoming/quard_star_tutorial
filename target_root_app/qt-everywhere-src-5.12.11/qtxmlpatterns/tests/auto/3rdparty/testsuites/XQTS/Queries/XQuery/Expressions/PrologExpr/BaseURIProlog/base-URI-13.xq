(: Name: base-URI-13 :)
(: Description: Evaluates base-uri property can contain an escape apostrophe. Used with static-base--uri function.:)

declare base-uri 'http://www.example.com/abc''';

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())