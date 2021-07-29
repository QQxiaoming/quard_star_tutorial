(:*******************************************************:)
(:Test: day-from-date-18                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "day-from-date" function        :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:day-from-date(xs:date("1970-01-01Z")) le fn:day-from-date(xs:date("1970-01-02Z"))