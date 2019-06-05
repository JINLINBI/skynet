drop table if exists `contact`;
create table `contact`(
    id int(11) not null auto_increment,
    user_id int(11) not null,
    friend_id int(11) not null,
    create_time timestamp not null default current_timestamp,
    update_time timestamp not null default current_timestamp,
    primary key(`id`, `user_id`, `friend_id`)
)