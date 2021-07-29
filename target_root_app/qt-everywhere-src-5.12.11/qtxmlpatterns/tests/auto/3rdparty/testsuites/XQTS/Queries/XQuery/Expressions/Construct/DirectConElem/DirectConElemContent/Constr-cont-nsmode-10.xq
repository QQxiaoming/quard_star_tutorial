(:*******************************************************:)
(:Name: Constr-cont-nsmode-10                            :)
(:Written By: Nicolae Brinza                             :)
(:Purpose: Copy namespace sensitive content              :)
(:*******************************************************:)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element { "new" } {
  $input-context//*:QName/@*
}