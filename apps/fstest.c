#include <xinu.h>
#include <fs.h>


char test_text[7672] = "Drawings me opinions returned absolute in. Otherwise therefore sex did are unfeeling something. Certain be ye amiable by exposed so. To celebrated estimating excellence do. Coming either suffer living her gay theirs. Furnished do otherwise daughters contented conveying attempted no. Was yet general visitor present hundred too brother fat arrival. Friend are day own either lively new. Did shy say mention enabled through elderly improve. As at so believe account evening behaved hearted is. House is tiled we aware. It ye greatest removing concerns an overcame appetite. Manner result square father boy behind its his. Their above spoke match ye mr right oh as first. Be my depending to believing perfectly concealed household. Point could to built no hours smile sense. Not far stuff she think the jokes. Going as by do known noise he wrote round leave. Warmly put branch people narrow see. Winding its waiting yet parlors married own feeling. Marry fruit do spite jokes an times. Whether at it unknown warrant herself winding if. Him same none name sake had post love. An busy feel form hand am up help. Parties it brother amongst an fortune of. Twenty behind wicket why age now itself ten. Style never met and those among great. At no or september sportsmen he perfectly happiness attending. Depending listening delivered off new she procuring satisfied sex existence. Person plenty answer to exeter it if. Law use assistance especially resolution cultivated did out sentiments unsatiable. Way necessary had intention happiness but september delighted his curiosity. Furniture furnished or on strangers neglected remainder engrossed. He as compliment unreserved projecting. Between had observe pretend delight for believe. Do newspaper questions consulted sweetness do. Our sportsman his unwilling fulfilled departure law. Now world own total saved above her cause table. Wicket myself her square remark the should far secure sex. Smiling cousins warrant law explain for whether. Tiled say decay spoil now walls meant house. My mr interest thoughts screened of outweigh removing. Evening society musical besides inhabit ye my. Lose hill well up will he over on. Increasing sufficient everything men him admiration unpleasing sex. Around really his use uneasy longer him man. His our pulled nature elinor talked now for excuse result. Admitted add peculiar get joy doubtful. Chapter too parties its letters nor. Cheerful but whatever ladyship disposed yet judgment. Lasted answer oppose to ye months no esteem. Branched is on an ecstatic directly it. Put off continue you denoting returned juvenile. Looked person sister result mr to. Replied demands charmed do viewing ye colonel to so. Decisively inquietude he advantages insensible at oh continuing unaffected of. Admiration stimulated cultivated reasonable be projection possession of. Real no near room ye bred sake if some. Is arranging furnished knowledge agreeable so. Fanny as smile up small. It vulgar chatty simple months turned oh at change of. Astonished set expression solicitude way admiration. Do in laughter securing smallest sensible no mr hastened. As perhaps proceed in in brandon of limited unknown greatly. Distrusts fulfilled happiness unwilling as explained of difficult. No landlord of peculiar ladyship attended if contempt ecstatic. Loud wish made on is am as hard. Court so avoid in plate hence. Of received mr breeding concerns peculiar securing landlord. Spot to many it four bred soon well to. Or am promotion in no departure abilities. Whatever landlord yourself at by pleasure of children be. Same an quit most an. Admitting an mr disposing sportsmen. Tried on cause no spoil arise plate. Longer ladies valley get esteem use led six. Middletons resolution advantages expression themselves partiality so me at. West none hope if sing oh sent tell is.Drawings me opinions returned absolute in. Otherwise therefore sex did are unfeeling something. Certain be ye amiable by exposed so. To celebrated estimating excellence do. Coming either suffer living her gay theirs. Furnished do otherwise daughters contented conveying attempted no. Was yet general visitor present hundred too brother fat arrival. Friend are day own either lively new. Did shy say mention enabled through elderly improve. As at so believe account evening behaved hearted is. House is tiled we aware. It ye greatest removing concerns an overcame appetite. Manner result square father boy behind its his. Their above spoke match ye mr right oh as first. Be my depending to believing perfectly concealed household. Point could to built no hours smile sense. Not far stuff she think the jokes. Going as by do known noise he wrote round leave. Warmly put branch people narrow see. Winding its waiting yet parlors married own feeling. Marry fruit do spite jokes an times. Whether at it unknown warrant herself winding if. Him same none name sake had post love. An busy feel form hand am up help. Parties it brother amongst an fortune of. Twenty behind wicket why age now itself ten. Style never met and those among great. At no or september sportsmen he perfectly happiness attending. Depending listening delivered off new she procuring satisfied sex existence. Person plenty answer to exeter it if. Law use assistance especially resolution cultivated did out sentiments unsatiable. Way necessary had intention happiness but september delighted his curiosity. Furniture furnished or on strangers neglected remainder engrossed. He as compliment unreserved projecting. Between had observe pretend delight for believe. Do newspaper questions consulted sweetness do. Our sportsman his unwilling fulfilled departure law. Now world own total saved above her cause table. Wicket myself her square remark the should far secure sex. Smiling cousins warrant law explain for whether. Tiled say decay spoil now walls meant house. My mr interest thoughts screened of outweigh removing. Evening society musical besides inhabit ye my. Lose hill well up will he over on. Increasing sufficient everything men him admiration unpleasing sex. Around really his use uneasy longer him man. His our pulled nature elinor talked now for excuse result. Admitted add peculiar get joy doubtful. Chapter too parties its letters nor. Cheerful but whatever ladyship disposed yet judgment. Lasted answer oppose to ye months no esteem. Branched is on an ecstatic directly it. Put off continue you denoting returned juvenile. Looked person sister result mr to. Replied demands charmed do viewing ye colonel to so. Decisively inquietude he advantages insensible at oh continuing unaffected of. Admiration stimulated cultivated reasonable be projection possession of. Real no near room ye bred sake if some. Is arranging furnished knowledge agreeable so. Fanny as smile up small. It vulgar chatty simple months turned oh at change of. Astonished set expression solicitude way admiration. Do in laughter securing smallest sensible no mr hastened. As perhaps proceed in in brandon of limited unknown greatly. Distrusts fulfilled happiness unwilling as explained of difficult. No landlord of peculiar ladyship attended if contempt ecstatic. Loud wish made on is am as hard. Court so avoid in plate hence. Of received mr breeding concerns peculiar securing landlord. Spot to many it four bred soon well to. Or am promotion in no departure abilities. Whatever landlord yourself at by pleasure of children be. Same an quit most an. Admitting an mr disposing sportsmen. Tried on cause no spoil arise plate. Longer ladies valley get esteem use led six. Middletons resolution advantages expression themselves partiality so me at. West none hope if sing oh sent tell is.";

#ifdef FS

/**
 * TEST
 * MACRO to run a function and print whether it was successful or not
 */
#define TEST(function)                                         \
  if (nargs == 1 || strcmp(args[1], #function) == 0) {         \
    if ((function)() != SYSERR) {                              \
      printf("%-35s: [\033[32mPASS\033[39m]\n", #function);    \
    } else {                                                   \
      printf("%-35s: [\033[31mFAIL\033[39m]\n", #function);    \
    }                                                          \
  }


/* #define FSTEST_DEBUG */

/**
 * MACROs for unit testing
 */

/**
 * ASSERT_TEST
 * Run a function and report the return code
 */
#define ASSERT_TEST(function)                             printf("\033[33mTEST  %20s:%-3d %30s:\033[39m %d\n", __FILE__, __LINE__, #function, (function));

#ifdef FSTEST_DEBUG

#define ASSERT_TRUE(expr)     if (!(expr))              { printf("\033[31mERROR %20s:%-3d %30s()\033[39m '%s' != TRUE\n", __FILE__, __LINE__, __func__, #expr);       return SYSERR; }
#define ASSERT_PASS(function) if ((function) == SYSERR) { printf("\033[31mERROR %20s:%-3d %30s()\033[39m '%s' == SYSERR\n", __FILE__, __LINE__, __func__, #function); return SYSERR; }
#define ASSERT_FAIL(function) if ((function) != SYSERR) { printf("\033[31mERROR %20s:%-3d %30s()\033[39m '%s' != SYSERR\n", __FILE__, __LINE__, __func__, #function); return SYSERR; }

#else

/**
 * ASSERT_TRUE
 * Evaluate an expression and return SYSERR if false
 */
#define ASSERT_TRUE(expr)     if (!(expr))              { return SYSERR; }

/**
 * ASSERT_PASS
 * Run a function and return SYSERR if its return code is SYSERR
 */
#define ASSERT_PASS(function) if ((function) == SYSERR) { return SYSERR; }

/**
 * ASSERT_FAIL
 * Run a function and return SYSERR if its return code is not SYSERR
 */
#define ASSERT_FAIL(function) if ((function) != SYSERR) { return SYSERR; }

#endif


int fstest_testbitmask(void) {

  bs_mkdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS);
  fs_mkfs(0, DEFAULT_NUM_INODES);

  fs_setmaskbit(31);
  fs_setmaskbit(95);
  fs_setmaskbit(159);
  fs_setmaskbit(223);
  fs_setmaskbit(287);
  fs_setmaskbit(351);
  fs_setmaskbit(415);
  fs_setmaskbit(479);
  fs_setmaskbit(90);
  fs_setmaskbit(154);
  fs_setmaskbit(218);
  fs_setmaskbit(282);
  fs_setmaskbit(346);
  fs_setmaskbit(347);
  fs_setmaskbit(348);
  fs_setmaskbit(349);
  fs_setmaskbit(350);
  fs_setmaskbit(100);
  fs_setmaskbit(164);
  fs_setmaskbit(228);
  fs_setmaskbit(292);
  fs_setmaskbit(356);
  fs_setmaskbit(355);
  fs_setmaskbit(354);
  fs_setmaskbit(353);
  fs_setmaskbit(352);

  fs_printfreemask();

  fs_clearmaskbit(31);
  fs_clearmaskbit(95);
  fs_clearmaskbit(159);
  fs_clearmaskbit(223);
  fs_clearmaskbit(287);
  fs_clearmaskbit(351);
  fs_clearmaskbit(415);
  fs_clearmaskbit(479);
  fs_clearmaskbit(90);
  fs_clearmaskbit(154);
  fs_clearmaskbit(218);
  fs_clearmaskbit(282);
  fs_clearmaskbit(346);
  fs_clearmaskbit(347);
  fs_clearmaskbit(348);
  fs_clearmaskbit(349);
  fs_clearmaskbit(350);
  fs_clearmaskbit(100);
  fs_clearmaskbit(164);
  fs_clearmaskbit(228);
  fs_clearmaskbit(292);
  fs_clearmaskbit(356);
  fs_clearmaskbit(355);
  fs_clearmaskbit(354);
  fs_clearmaskbit(353);
  fs_clearmaskbit(352);

  fs_printfreemask();

  fs_freefs(0);
  bs_freedev(0);

  return OK;
}

/**
 * Try to
 * - (re-)create and free the block device,
 * - (re-)create and free the file system, and
 * - create your first file "test"
 */
int fstest_mkdev() {

  int i;

  for (i = 0; i < 10; i++) {
    ASSERT_PASS(bs_mkdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS))
    ASSERT_PASS(fs_mkfs(0, DEFAULT_NUM_INODES))
    /* ASSERT_PASS(fs_create("test", O_CREAT)) */
    printf("%d\n", fs_create("test", O_CREAT));
    ASSERT_PASS(fs_freefs(0))
    ASSERT_PASS(bs_freedev(0))
  }

  return OK;
}

int fstest_create_close_open() {

  int i, fd;

  for (i = 0; i < 1; i++) {
    ASSERT_PASS(bs_mkdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS))
    ASSERT_PASS(fs_mkfs(0, DEFAULT_NUM_INODES))
    /* ASSERT_PASS(fs_create("test", O_CREAT)) */
    fd = fs_create("test", O_CREAT);
    printf("%d\n", fd);
    fs_close(fd);
    fd = fs_open("test", O_RDWR);
    printf("%d\n", fd);
    ASSERT_PASS(fs_freefs(0))
    ASSERT_PASS(bs_freedev(0))
  }

  return OK;
}

/* int fstest_read_write_seek() { */

/*   int i, fd; */

/*   for (i = 0; i < 1; i++) { */
/*     ASSERT_PASS(bs_mkdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS)) */
/*     ASSERT_PASS(fs_mkfs(0, DEFAULT_NUM_INODES)) */
/*     /1* ASSERT_PASS(fs_create("test", O_CREAT)) *1/ */
/*     fd = fs_create("test", O_CREAT); */
/*     fs_print_inode(fd); */
/*     fs_printfreemask(); */
/*     printf("%d\n", fd); */
/*     char data[128] = "abcdef"; */
/*     fs_print_oft(); */
/*     printf("written %d\n", fs_write(fd, data, 6)); */
/*     fs_printfreemask(); */
/*     fs_print_inode(fd); */
/*     printf("return %d\n", fs_seek(fd, 2)); */
/*     fs_print_inode(fd); */
/*     data[0] = '\0'; */
/*     printf("read %d\n",fs_read(fd, data, 6)); */
/*     fs_print_inode(fd); */
/*     printf("data retrived %s\n",data); */
/*     fs_close(fd); */
/*     ASSERT_PASS(fs_freefs(0)) */
/*     ASSERT_PASS(bs_freedev(0)) */
/*   } */

/*   return OK; */
/* } */

int fstest_read_write_seek() {

  int i, fd;

  for (i = 0; i < 1; i++) {
    ASSERT_PASS(bs_mkdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS))
    ASSERT_PASS(fs_mkfs(0, DEFAULT_NUM_INODES))
    fd = fs_create("test", O_CREAT);
    fs_printfreemask();
    fs_print_inode(fd);
    printf("opened fd is %d\n", fd);
    printf("written %d\n", fs_write(fd, test_text, 7672));
    fs_printfreemask();
    fs_print_inode(fd);
    

    char data1[4096];
    /* Read first 100 */
    printf("seek 0 return %d\n", fs_seek(fd, 0));
    printf("read %d\n",fs_read(fd, data1, 100));
    printf("data retrived:\n%s\n",data1);
    fs_print_inode(fd);

    char data2[4096];
    /* Read more that present */
    printf("seek 0 return %d\n", fs_seek(fd, 0));
    printf("read %d\n",fs_read(fd, data2, 4000));
    printf("data retrived:\n%s\n",data2);
    fs_print_inode(fd);

    char data3[4096];
    /* Read from middle between boundaries  */
    printf("seek 3536 return %d\n", fs_seek(fd, 3536));
    printf("read %d\n",fs_read(fd, data3, 200));
    printf("data retrived:\n%s\n",data3);
    fs_print_inode(fd);

    char data4[4096];
    printf("seek 3536 return %d\n", fs_seek(fd, 10));
    printf("read %d\n",fs_read(fd, data4, 10));
    printf("data retrived:\n%s\n",data4);
    /* char data4[4096]; */
    /* printf("seek 3536 return %d\n", fs_seek(fd, 10)); */
    printf("read %d\n",fs_read(fd, data4, 10));
    printf("data retrived:\n%s\n",data4);
    fs_print_inode(fd);
    fs_close(fd);
    ASSERT_PASS(fs_freefs(0))
    ASSERT_PASS(bs_freedev(0))
  }

  return OK;
}

int fstest_create_close_create() {

  int i, fd;

  for (i = 0; i < 1; i++) {
    ASSERT_PASS(bs_mkdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS))
    ASSERT_PASS(fs_mkfs(0, DEFAULT_NUM_INODES))
    /* ASSERT_PASS(fs_create("test", O_CREAT)) */
    fd = fs_create("test", O_CREAT);
    printf("%d\n", fd);
    fs_close(fd);
    fd = fs_create("test", O_CREAT);
    printf("%d\n", fd);
    ASSERT_PASS(fs_freefs(0))
    ASSERT_PASS(bs_freedev(0))
  }

  return OK;
}

#endif


int fstest(int nargs, char *args[]) {

  /* Output help, if '--help' argument was supplied */
  if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
    printf("Usage: %s [TEST]\n\n", args[0]);
    printf("Description:\n");
    printf("\tFilesystem Test\n");
    printf("Options:\n");
    printf("\t--help\tdisplay this help and exit\n");
    return OK;
  }

  /* Check for correct number of arguments */
  if (nargs > 2) {
    fprintf(stderr, "%s: too many arguments\n", args[0]);
    fprintf(stderr, "Try '%s --help' for more information\n",
            args[0]);
    return SYSERR;
  }

#ifdef FS

  printf("\n\n\n");
  TEST(fstest_testbitmask)
  TEST(fstest_mkdev)
  TEST(fstest_create_close_open)
  TEST(fstest_create_close_create)
  TEST(fstest_read_write_seek)

#else
  printf("No filesystem support\n");
#endif

  return OK;
}
