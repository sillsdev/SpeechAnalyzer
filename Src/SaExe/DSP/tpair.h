struct tpair {
    typedef First first_type;
    typedef Second second_type;

    First first;
    Second second;

    tpair();
    tpair(const First & x, const Second & y);
};

int operator <(const tpair & x, const tpair & y);
